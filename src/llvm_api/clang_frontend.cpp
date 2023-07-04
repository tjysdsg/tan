// Based on https://github.com/llvm/llvm-project/blob/main/clang/tools/driver/driver.cpp

//===-- driver.cpp - Clang GCC-Compatible Driver --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "tan/tan.h"
#include <iostream>

#include <clang/Driver/Driver.h>
#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/Stack.h>
#include <clang/Driver/Compilation.h>
#include <clang/Driver/ToolChain.h>
#include <clang/Frontend/ChainedDiagnosticConsumer.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Frontend/SerializedDiagnosticPrinter.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/Utils.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/CrashRecoveryContext.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/StringSaver.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>

using namespace clang;
using namespace clang::driver;
using namespace llvm::opt;

str GetExecutablePath(const str &name) {
  auto clang_or_err = llvm::sys::findProgramByName(name, {}); // find executable in PATH
  if (!clang_or_err) {
    std::cerr << "Cannot find clang executable: " << clang_or_err.getError() << "\n";
    exit(1);
  }
  return clang_or_err.get();
}

static const char *GetStableCStr(std::set<str> &SavedStrings, StringRef S) {
  return SavedStrings.insert(std::string(S)).first->c_str();
}

static void insertTargetAndModeArgs(const ParsedClangName &NameParts, SmallVectorImpl<const char *> &ArgVector,
                                    std::set<str> &SavedStrings) {
  // Put target and mode arguments at the start of argument list so that
  // arguments specified in command line could override them. Avoid putting
  // them at index 0, as an option like '-cc1' must remain the first.
  int InsertionPoint = 0;
  if (ArgVector.size() > 0) {
    ++InsertionPoint;
  }

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
  if (ExeBasename.equals_insensitive("cl")) {
    ExeBasename = "clang-cl";
  }
  DiagClient->setPrefix(std::string(ExeBasename));
}

/**
 * \brief Call clang with commandline args
 * \details Requires the system to have `clang` in $PATH.
 *          This function doesn't directly invoke clang,
 *          but it uses the binary path to find paths to standard headers and libraries.
 *          (Try replacing $(which clang) with a blank text file and see if the compiler works :D)
 *  Copied from https://github.com/llvm/llvm-project/blob/main/clang/tools/driver/driver.cpp
 */
int clang_main(int argc_, const char **argv_) {
  for (int i = 0; i < argc_; ++i) {
    printf("%s ", argv_[i]);
  }
  printf("\n");

  llvm::cl::ResetCommandLineParser();

  noteBottomOfStack();

  // FIXME: previously passed-in arguments will interfere with the cmd parser on Windows
  int fake_argc = argc_;
  const char **fake_argv = argv_;
  llvm::InitLLVM X(fake_argc, fake_argv);

  SmallVector<const char *, 256> argv(argv_, argv_ + argc_);
  if (llvm::sys::Process::FixupStandardFileDescriptors()) {
    return 1;
  }

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
    SmallVector<pair<int, const Command *>, 4> FailingCommands;
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

using tanlang::TanCompilation;

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
  str opt_level = opt_level_to_string(config->opt_level);
  args.push_back(opt_level.c_str());
  args.push_back("-c");

  return clang_main((int)args.size(), args.data());
}
