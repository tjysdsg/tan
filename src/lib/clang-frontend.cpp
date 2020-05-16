// 2020 modified by Jiyang Tang

//===-- driver.cpp - Clang GCC-Compatible Driver --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "src/llvm_include.h"
#include "src/lib/misc.h"
#include "base.h"
#include <iostream>
#include "libtanc.h"
using namespace clang;
using namespace clang::driver;
using namespace llvm::opt;

str GetExecutablePath(str name) {
  auto clang_or_err = llvm::sys::findProgramByName(name, {__STR__(LLVM_BIN_DIR)});
  if (!clang_or_err) {
    std::cerr << "Cannot find clang executable: " << clang_or_err.getError() << "\n";
    exit(0);
  }
  return clang_or_err.get();
}

static const char *GetStableCStr(std::set<str> &SavedStrings, StringRef S) {
  return SavedStrings.insert(S).first->c_str();
}

static void insertTargetAndModeArgs(const ParsedClangName &NameParts,
    SmallVectorImpl<const char *> &ArgVector,
    std::set<str> &SavedStrings) {
  // Put target and mode arguments at the start of argument list so that
  // arguments specified in command line could override them. Avoid putting
  // them at index 0, as an option like '-cc1' must remain the first.
  int InsertionPoint = 0;
  if (ArgVector.size() > 0) { ++InsertionPoint; }

  if (NameParts.DriverMode) {
    // Add the mode flag to the arguments.
    ArgVector.insert(ArgVector.begin() + InsertionPoint, GetStableCStr(SavedStrings, NameParts.DriverMode));
  }

  if (NameParts.TargetIsValid) {
    const char *arr[] = {"-target", GetStableCStr(SavedStrings, NameParts.TargetPrefix)};
    ArgVector.insert(ArgVector.begin() + InsertionPoint, std::begin(arr), std::end(arr));
  }
}

static void SetBackdoorDriverOutputsFromEnvVars(Driver &TheDriver) {
  // Handle CC_PRINT_OPTIONS and CC_PRINT_OPTIONS_FILE.
  TheDriver.CCPrintOptions = ::getenv("CC_PRINT_OPTIONS") != nullptr;
  if (TheDriver.CCPrintOptions) {
    TheDriver.CCPrintOptionsFilename = ::getenv("CC_PRINT_OPTIONS_FILE");
  }

  // Handle CC_PRINT_HEADERS and CC_PRINT_HEADERS_FILE.
  TheDriver.CCPrintHeaders = ::getenv("CC_PRINT_HEADERS") != nullptr;
  if (TheDriver.CCPrintHeaders) {
    TheDriver.CCPrintHeadersFilename = ::getenv("CC_PRINT_HEADERS_FILE");
  }

  // Handle CC_LOG_DIAGNOSTICS and CC_LOG_DIAGNOSTICS_FILE.
  TheDriver.CCLogDiagnostics = ::getenv("CC_LOG_DIAGNOSTICS") != nullptr;
  if (TheDriver.CCLogDiagnostics) {
    TheDriver.CCLogDiagnosticsFilename = ::getenv("CC_LOG_DIAGNOSTICS_FILE");
  }
}

static void FixupDiagPrefixExeName(TextDiagnosticPrinter *DiagClient, const str &Path) {
  // If the clang binary happens to be named cl.exe for compatibility reasons,
  // use clang-cl.exe as the prefix to avoid confusion between clang and MSVC.
  StringRef ExeBasename(llvm::sys::path::stem(Path));
  if (ExeBasename.equals_lower("cl")) {
    ExeBasename = "clang-cl";
  }
  DiagClient->setPrefix(ExeBasename);
}

// this lets us create the DiagnosticsEngine with a properly-filled-out DiagnosticOptions instance
static DiagnosticOptions *CreateAndPopulateDiagOpts(ArrayRef<const char *> argv) {
  auto *DiagOpts = new DiagnosticOptions;
  unsigned MissingArgIndex, MissingArgCount;
  InputArgList Args = getDriverOptTable().ParseArgs(argv.slice(1), MissingArgIndex, MissingArgCount);
  // We ignore MissingArgCount and the return value of ParseDiagnosticArgs.
  // Any errors that would be diagnosed here will also be diagnosed later,
  // when the DiagnosticsEngine actually exists.
  (void) ParseDiagnosticArgs(*DiagOpts, Args);
  return DiagOpts;
}

int main0(int argc_, const char **argv_) {
  noteBottomOfStack();
  llvm::InitLLVM X(argc_, argv_);
  SmallVector<const char *, 256> argv(argv_, argv_ + argc_);
  if (llvm::sys::Process::FixupStandardFileDescriptors()) { return 1; }

  llvm::InitializeAllTargets();
  auto TargetAndMode = ToolChain::getTargetAndModeFromProgramName(argv[0]);

  llvm::BumpPtrAllocator A;
  llvm::StringSaver Saver(A);

  llvm::cl::TokenizerCallback Tokenizer = &llvm::cl::TokenizeGNUCommandLine;
  llvm::cl::ExpandResponseFiles(Saver, Tokenizer, argv, false);

  std::set<str> SavedStrings;
  str Path = GetExecutablePath(argv[0]);
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = CreateAndPopulateDiagOpts(argv);
  TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
  FixupDiagPrefixExeName(DiagClient, Path);
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  if (!DiagOpts->DiagnosticSerializationFile.empty()) {
    auto SerializedConsumer = clang::serialized_diags::create(DiagOpts->DiagnosticSerializationFile, &*DiagOpts, true);
    Diags.setClient(new ChainedDiagnosticConsumer(Diags.takeClient(), std::move(SerializedConsumer)));
  }

  ProcessWarningOptions(Diags, *DiagOpts, false);

  Driver TheDriver(Path, llvm::sys::getDefaultTargetTriple(), Diags);
  TheDriver.setTargetAndMode(TargetAndMode);
  insertTargetAndModeArgs(TargetAndMode, argv, SavedStrings);
  SetBackdoorDriverOutputsFromEnvVars(TheDriver);

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(argv));
  int Res = 1;
  if (C && !C->containsError()) {
    SmallVector<std::pair<int, const Command *>, 4> FailingCommands;
    Res = TheDriver.ExecuteCompilation(*C, FailingCommands);

    for (const auto &P : FailingCommands) {
      int CommandRes = P.first;
      const Command *FailingCommand = P.second;
      if (!Res) {
        Res = CommandRes;
      }

      // If result status is < 0, then the driver command signalled an error.
      // If result status is 70, then the driver command reported a fatal error.
      // On Windows, abort will return an exit code of 3.  In these cases,
      // generate additional diagnostic information if possible.
      bool DiagnoseCrash = CommandRes < 0 || CommandRes == 70;
      #ifdef _WIN32
      DiagnoseCrash |= CommandRes == 3;
      #endif
      if (DiagnoseCrash) {
        TheDriver.generateCompilationDiagnostics(*C, *FailingCommand);
        break;
      }
    }
  }
  Diags.getClient()->finish();

  #ifdef _WIN32
  // Exit status should not be negative on Win32, unless abnormal termination.
  // Once abnormal termination was caught, negative status should not be
  // propagated.
  if (Res < 0)
    Res = 1;
  #endif

  /// if we have multiple failing commands, we return the result of the first failing command
  return Res;
}

int clang_main(int argc, const char **argv) {
  auto ret = main0(argc, argv);
  llvm::cl::ResetCommandLineParser();
  return ret;
}

int clang_compile(vector<str> input_files, TanCompilation *config) {
  vector<const char *> args;
  size_t n_import = config->import_dirs.size();
  args.reserve(input_files.size() + 2 * +1);
  args.push_back("clang");
  std::for_each(input_files.begin(), input_files.end(), [&args](const auto &s) { args.push_back(s.c_str()); });
  for (size_t i = 0; i < n_import; ++i) {
    args.push_back("-I");
    args.push_back(config->import_dirs[i].c_str());
  }
  args.push_back(opt_level_to_string(config->opt_level).c_str());
  args.push_back("-c");

  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_id(new clang::DiagnosticIDs());
  auto diag_options = new clang::DiagnosticOptions();
  clang::DiagnosticsEngine
      diag_engine(diag_id, diag_options, new clang::TextDiagnosticPrinter(llvm::errs(), diag_options));
  clang::driver::Driver driver(GetExecutablePath("clang"), llvm::sys::getDefaultTargetTriple(), diag_engine);

  auto *compilation = driver.BuildCompilation(args);
  SmallVector<std::pair<int, const Command *>, 0> failing_commands;
  if (compilation) { return driver.ExecuteCompilation(*compilation, failing_commands); }
  return 1;
}
