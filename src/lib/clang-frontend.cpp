//===-- driver.cpp - Clang GCC-Compatible Driver --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "src/llvm_include.h"
#include "base.h"
#include <iostream>
#include "libtanc.h"
#include <memory>
#include <set>
#include <system_error>
using namespace clang;
using namespace clang::driver;
using namespace llvm::opt;

str GetExecutablePath(const char *Argv0, bool CanonicalPrefixes) {
  if (!CanonicalPrefixes) {
    SmallString<128> ExecutablePath(Argv0);
    // Do a PATH lookup if Argv0 isn't a valid path.
    if (!llvm::sys::fs::exists(ExecutablePath)) {
      if (llvm::ErrorOr<str> P = llvm::sys::findProgramByName(ExecutablePath)) {
        ExecutablePath = *P;
      }
    }
    return ExecutablePath.str();
  }

  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *P = (void *) (intptr_t) GetExecutablePath;
  return llvm::sys::fs::getMainExecutable(Argv0, P);
}

static const char *GetStableCStr(std::set<str> &SavedStrings, StringRef S) {
  return SavedStrings.insert(S).first->c_str();
}

/// ApplyQAOverride - Apply a list of edits to the input argument lists.
///
/// The input string is a space separate list of edits to perform,
/// they are applied in order to the input argument lists. Edits
/// should be one of the following forms:
///
///  '#': Silence information about the changes to the command line arguments.
///
///  '^': Add FOO as a new argument at the beginning of the command line.
///
///  '+': Add FOO as a new argument at the end of the command line.
///
///  's/XXX/YYY/': Substitute the regular expression XXX with YYY in the command
///  line.
///
///  'xOPTION': Removes all instances of the literal argument OPTION.
///
///  'XOPTION': Removes all instances of the literal argument OPTION,
///  and the following argument.
///
///  'Ox': Removes all flags matching 'O' or 'O[sz0-9]' and adds 'Ox'
///  at the end of the command line.
///
/// \param OS - The stream to write edit information to.
/// \param Args - The vector of command line arguments.
/// \param Edit - The override command to perform.
/// \param SavedStrings - Set to use for storing string representations.
static void ApplyOneQAOverride(raw_ostream &OS,
    SmallVectorImpl<const char *> &Args,
    StringRef Edit,
    std::set<str> &SavedStrings) {
  // This does not need to be efficient.
  if (Edit[0] == '^') {
    const char *Str = GetStableCStr(SavedStrings, Edit.substr(1));
    OS << "### Adding argument " << Str << " at beginning\n";
    Args.insert(Args.begin() + 1, Str);
  } else if (Edit[0] == '+') {
    const char *Str = GetStableCStr(SavedStrings, Edit.substr(1));
    OS << "### Adding argument " << Str << " at end\n";
    Args.push_back(Str);
  } else if (Edit[0] == 's' && Edit[1] == '/' && Edit.endswith("/")
      && Edit.slice(2, Edit.size() - 1).find('/') != StringRef::npos) {
    StringRef MatchPattern = Edit.substr(2).split('/').first;
    StringRef ReplPattern = Edit.substr(2).split('/').second;
    ReplPattern = ReplPattern.slice(0, ReplPattern.size() - 1);

    for (unsigned i = 1, e = (unsigned) Args.size(); i != e; ++i) {
      // Ignore end-of-line response file markers
      if (Args[i] == nullptr) {
        continue;
      }
      str Repl = llvm::Regex(MatchPattern).sub(ReplPattern, Args[i]);

      if (Repl != Args[i]) {
        OS << "### Replacing '" << Args[i] << "' with '" << Repl << "'\n";
        Args[i] = GetStableCStr(SavedStrings, Repl);
      }
    }
  } else if (Edit[0] == 'x' || Edit[0] == 'X') {
    auto Option = Edit.substr(1);
    for (unsigned i = 1; i < Args.size();) {
      if (Option == Args[i]) {
        OS << "### Deleting argument " << Args[i] << '\n';
        Args.erase(Args.begin() + i);
        if (Edit[0] == 'X') {
          if (i < Args.size()) {
            OS << "### Deleting argument " << Args[i] << '\n';
            Args.erase(Args.begin() + i);
          } else {
            OS << "### Invalid X edit, end of command line!\n";
          }
        }
      } else {
        ++i;
      }
    }
  } else if (Edit[0] == 'O') {
    for (unsigned i = 1; i < Args.size();) {
      const char *A = Args[i];
      // Ignore end-of-line response file markers
      if (A == nullptr) {
        continue;
      }
      if (A[0] == '-' && A[1] == 'O'
          && (A[2] == '\0' || (A[3] == '\0' && (A[2] == 's' || A[2] == 'z' || ('0' <= A[2] && A[2] <= '9'))))) {
        OS << "### Deleting argument " << Args[i] << '\n';
        Args.erase(Args.begin() + i);
      } else {
        ++i;
      }
    }
    OS << "### Adding argument " << Edit << " at end\n";
    Args.push_back(GetStableCStr(SavedStrings, '-' + Edit.str()));
  } else {
    OS << "### Unrecognized edit: " << Edit << "\n";
  }
}

/// ApplyQAOverride - Apply a comma separate list of edits to the
/// input argument lists. See ApplyOneQAOverride.
static void ApplyQAOverride(SmallVectorImpl<const char *> &Args, const char *OverrideStr, std::set<str> &SavedStrings) {
  raw_ostream *OS = &llvm::errs();

  if (OverrideStr[0] == '#') {
    ++OverrideStr;
    OS = &llvm::nulls();
  }

  *OS << "### CCC_OVERRIDE_OPTIONS: " << OverrideStr << "\n";

  // This does not need to be efficient.

  const char *S = OverrideStr;
  while (*S) {
    const char *End = ::strchr(S, ' ');
    if (!End) {
      End = S + strlen(S);
    }
    if (End != S) {
      ApplyOneQAOverride(*OS, Args, str(S, End), SavedStrings);
    }
    S = End;
    if (*S != '\0') {
      ++S;
    }
  }
}

extern int cc1_main(ArrayRef<const char *> Argv, const char *Argv0, void *MainAddr);
extern int cc1as_main(ArrayRef<const char *> Argv, const char *Argv0, void *MainAddr);

static void insertTargetAndModeArgs(const ParsedClangName &NameParts,
    SmallVectorImpl<const char *> &ArgVector,
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

static void getCLEnvVarOptions(str &EnvValue, llvm::StringSaver &Saver, SmallVectorImpl<const char *> &Opts) {
  llvm::cl::TokenizeWindowsCommandLine(EnvValue, Saver, Opts);
  // The first instance of '#' should be replaced with '=' in each option.
  for (const char *Opt : Opts) {
    if (char *NumberSignPtr = const_cast<char *>(::strchr(Opt, '#'))) {
      *NumberSignPtr = '=';
    }
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

// This lets us create the DiagnosticsEngine with a properly-filled-out
// DiagnosticOptions instance.
static DiagnosticOptions *CreateAndPopulateDiagOpts(ArrayRef<const char *> argv, bool &UseNewCC1Process) {
  auto *DiagOpts = new DiagnosticOptions;
  unsigned MissingArgIndex, MissingArgCount;
  InputArgList Args = getDriverOptTable().ParseArgs(argv.slice(1), MissingArgIndex, MissingArgCount);
  // We ignore MissingArgCount and the return value of ParseDiagnosticArgs.
  // Any errors that would be diagnosed here will also be diagnosed later,
  // when the DiagnosticsEngine actually exists.
  (void) ParseDiagnosticArgs(*DiagOpts, Args);
  UseNewCC1Process =
      Args.hasFlag(clang::driver::options::OPT_fno_integrated_cc1, clang::driver::options::OPT_fintegrated_cc1,
          /*Default=*/CLANG_SPAWN_CC1);
  return DiagOpts;
}

static void SetInstallDir(SmallVectorImpl<const char *> &argv, Driver &TheDriver, bool CanonicalPrefixes) {
  // Attempt to find the original path used to invoke the driver, to determine
  // the installed path. We do this manually, because we want to support that
  // path being a symlink.
  SmallString<128> InstalledPath(argv[0]);

  // Do a PATH lookup, if there are no directory components.
  if (llvm::sys::path::filename(InstalledPath) == InstalledPath) {
    if (llvm::ErrorOr<str> Tmp = llvm::sys::findProgramByName(llvm::sys::path::filename(InstalledPath.str()))) {
      InstalledPath = *Tmp;
    }
  }

  // FIXME: We don't actually canonicalize this, we just make it absolute.
  if (CanonicalPrefixes) {
    llvm::sys::fs::make_absolute(InstalledPath);
  }

  StringRef InstalledPathParent(llvm::sys::path::parent_path(InstalledPath));
  if (llvm::sys::fs::exists(InstalledPathParent)) {
    TheDriver.setInstalledDir(InstalledPathParent);
  }
}

static int ExecuteCC1Tool(SmallVectorImpl<const char *> &ArgV) {
  // If we call the cc1 tool from the clangDriver library (through
  // Driver::CC1Main), we need to clean up the options usage count. The options
  // are currently global, and they might have been used previously by the
  // driver.
  llvm::cl::ResetAllOptionOccurrences();

  llvm::BumpPtrAllocator A;
  llvm::StringSaver Saver(A);
  llvm::cl::ExpandResponseFiles(Saver, &llvm::cl::TokenizeGNUCommandLine, ArgV,
      /*MarkEOLs=*/false);
  StringRef Tool = ArgV[1];
  void *GetExecutablePathVP = (void *) (intptr_t) GetExecutablePath;
  if (Tool == "-cc1") {
    return cc1_main(makeArrayRef(ArgV).slice(2), ArgV[0], GetExecutablePathVP);
  }
  if (Tool == "-cc1as") {
    return cc1as_main(makeArrayRef(ArgV).slice(2), ArgV[0], GetExecutablePathVP);
  }
  // Reject unknown tools.
  llvm::errs() << "error: unknown integrated tool '" << Tool << "'. " << "Valid tools include '-cc1' and '-cc1as'.\n";
  return 1;
}

int main0(int argc_, const char **argv_) {
  noteBottomOfStack();
  llvm::InitLLVM X(argc_, argv_);
  size_t argv_offset = (strcmp(argv_[1], "-cc1") == 0 || strcmp(argv_[1], "-cc1as") == 0) ? 0 : 1;
  SmallVector<const char *, 256> argv(argv_ + argv_offset, argv_ + argc_);

  if (llvm::sys::Process::FixupStandardFileDescriptors()) {
    return 1;
  }

  llvm::InitializeAllTargets();
  auto TargetAndMode = ToolChain::getTargetAndModeFromProgramName(argv[0]);

  llvm::BumpPtrAllocator A;
  llvm::StringSaver Saver(A);

  // Parse response files using the GNU syntax, unless we're in CL mode. There
  // are two ways to put clang in CL compatibility mode: argv[0] is either
  // clang-cl or cl, or --driver-mode=cl is on the command line. The normal
  // command line parsing can't happen until after response file parsing, so we
  // have to manually search for a --driver-mode=cl argument the hard way.
  // Finally, our -cc1 tools don't care which tokenization mode we use because
  // response files written by clang will tokenize the same way in either mode.
  bool ClangCLMode = false;
  if (StringRef(TargetAndMode.DriverMode).equals("--driver-mode=cl") || llvm::find_if(argv, [](const char *F) {
    return F && strcmp(F, "--driver-mode=cl") == 0;
  }) != argv.end()) {
    ClangCLMode = true;
  }
  enum { Default, POSIX, Windows } RSPQuoting = Default;
  for (const char *F : argv) {
    if (strcmp(F, "--rsp-quoting=posix") == 0) {
      RSPQuoting = POSIX;
    } else if (strcmp(F, "--rsp-quoting=windows") == 0) {
      RSPQuoting = Windows;
    }
  }

  // Determines whether we want nullptr markers in argv to indicate response
  // files end-of-lines. We only use this for the /LINK driver argument with
  // clang-cl.exe on Windows.
  bool MarkEOLs = ClangCLMode;

  llvm::cl::TokenizerCallback Tokenizer;
  if (RSPQuoting == Windows || (RSPQuoting == Default && ClangCLMode)) {
    Tokenizer = &llvm::cl::TokenizeWindowsCommandLine;
  } else {
    Tokenizer = &llvm::cl::TokenizeGNUCommandLine;
  }

  if (MarkEOLs && argv.size() > 1 && StringRef(argv[1]).startswith("-cc1")) {
    MarkEOLs = false;
  }
  llvm::cl::ExpandResponseFiles(Saver, Tokenizer, argv, MarkEOLs);

  // Handle -cc1 integrated tools, even if -cc1 was expanded from a response
  // file.
  auto FirstArg = std::find_if(argv.begin() + 1, argv.end(), [](const char *A) { return A != nullptr; });
  if (FirstArg != argv.end() && StringRef(*FirstArg).startswith("-cc1")) {
    // If -cc1 came from a response file, remove the EOL sentinels.
    if (MarkEOLs) {
      auto newEnd = std::remove(argv.begin(), argv.end(), nullptr);
      argv.resize((size_t) (newEnd - argv.begin()));
    }
    return ExecuteCC1Tool(argv);
  }

  // Handle options that need handling before the real command line parsing in
  // Driver::BuildCompilation()
  bool CanonicalPrefixes = true;
  for (size_t i = 1, size = argv.size(); i < size; ++i) {
    // Skip end-of-line response file markers
    if (argv[i] == nullptr) {
      continue;
    }
    if (StringRef(argv[i]) == "-no-canonical-prefixes") {
      CanonicalPrefixes = false;
      break;
    }
  }

  // Handle CL and _CL_ which permits additional command line options to be
  // prepended or appended.
  if (ClangCLMode) {
    // Arguments in "CL" are prepended.
    llvm::Optional<str> OptCL = llvm::sys::Process::GetEnv("CL");
    if (OptCL.hasValue()) {
      SmallVector<const char *, 8> PrependedOpts;
      getCLEnvVarOptions(OptCL.getValue(), Saver, PrependedOpts);

      // Insert right after the program name to prepend to the argument list.
      argv.insert(argv.begin() + 1, PrependedOpts.begin(), PrependedOpts.end());
    }
    // Arguments in "_CL_" are appended.
    llvm::Optional<str> Opt_CL_ = llvm::sys::Process::GetEnv("_CL_");
    if (Opt_CL_.hasValue()) {
      SmallVector<const char *, 8> AppendedOpts;
      getCLEnvVarOptions(Opt_CL_.getValue(), Saver, AppendedOpts);

      // Insert at the end of the argument list to append.
      argv.append(AppendedOpts.begin(), AppendedOpts.end());
    }
  }

  std::set<str> SavedStrings;
  // Handle CCC_OVERRIDE_OPTIONS, used for editing a command line behind the
  // scenes.
  if (const char *OverrideStr = ::getenv("CCC_OVERRIDE_OPTIONS")) {
    // FIXME: Driver shouldn't take extra initial argument.
    ApplyQAOverride(argv, OverrideStr, SavedStrings);
  }

  str Path = GetExecutablePath(argv[0], CanonicalPrefixes);

  // Whether the cc1 tool should be called inside the current process, or if we
  // should spawn a new clang subprocess (old behavior).
  // Not having an additional process saves some execution time of Windows,
  // and makes debugging and profiling easier.
  bool UseNewCC1Process;

  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = CreateAndPopulateDiagOpts(argv, UseNewCC1Process);

  TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
  FixupDiagPrefixExeName(DiagClient, Path);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());

  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  if (!DiagOpts->DiagnosticSerializationFile.empty()) {
    auto SerializedConsumer =
        clang::serialized_diags::create(DiagOpts->DiagnosticSerializationFile, &*DiagOpts, /*MergeChildRecords=*/true);
    Diags.setClient(new ChainedDiagnosticConsumer(Diags.takeClient(), std::move(SerializedConsumer)));
  }

  ProcessWarningOptions(Diags, *DiagOpts, /*ReportDiags=*/false);

  Driver TheDriver(Path, llvm::sys::getDefaultTargetTriple(), Diags);
  SetInstallDir(argv, TheDriver, CanonicalPrefixes);
  TheDriver.setTargetAndMode(TargetAndMode);

  insertTargetAndModeArgs(TargetAndMode, argv, SavedStrings);

  SetBackdoorDriverOutputsFromEnvVars(TheDriver);

  if (!UseNewCC1Process) {
    TheDriver.CC1Main = &ExecuteCC1Tool;
    // Ensure the CC1Command actually catches cc1 crashes
    llvm::CrashRecoveryContext::Enable();
  }

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(argv));
  int Res = 1;
  if (C && !C->containsError()) {
    SmallVector<std::pair<int, const Command *>, 4> FailingCommands;
    Res = TheDriver.ExecuteCompilation(*C, FailingCommands);

    // Force a crash to test the diagnostics.
    if (TheDriver.GenReproducer) {
      Diags.Report(diag::err_drv_force_crash) << !::getenv("FORCE_CLANG_DIAGNOSTICS_CRASH");

      // Pretend that every command failed.
      FailingCommands.clear();
      for (const auto &J : C->getJobs()) {
        if (const Command *C = dyn_cast<Command>(&J)) {
          FailingCommands.push_back(std::make_pair(-1, C));
        }
      }
    }

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

  // If any timers were active but haven't been destroyed yet, print their
  // results now.  This happens in -disable-free mode.
  llvm::TimerGroup::printAll(llvm::errs());
  llvm::TimerGroup::clearAll();

  #ifdef _WIN32
  // Exit status should not be negative on Win32, unless abnormal termination.
  // Once abnormal termination was caught, negative status should not be
  // propagated.
  if (Res < 0)
    Res = 1;
  #endif

  // If we have multiple failing commands, we return the result of the first
  // failing command.
  return Res;
}

int clang_main(int argc, const char **argv) {
  auto ret = main0(argc, argv);
  llvm::cl::ResetCommandLineParser();
  return ret;
}

int clang_compile(vector<const char *> input_files, TanCompilation *config) {
  vector<const char *> args;
  args.reserve(input_files.size() + 2 * config->n_import_dirs + 1);
  args.push_back("clang");
  args.insert(args.end(), input_files.begin(), input_files.end());
  for (size_t i = 0; i < config->n_import_dirs; ++i) {
    args.push_back("-I");
    args.push_back(config->import_dirs[i]);
  }
  args.push_back("-c");

  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> diag_id(new clang::DiagnosticIDs());
  auto diag_options = new clang::DiagnosticOptions();
  clang::DiagnosticsEngine
      diag_engine(diag_id, diag_options, new clang::TextDiagnosticPrinter(llvm::errs(), diag_options));
  auto clang_or_err = llvm::sys::findProgramByName("clang", {__STR__(LLVM_BIN_DIR)});
  if (!clang_or_err) {
    std::cerr << "Cannot find clang executable: " << clang_or_err.getError() << "\n";
    return 1;
  }
  clang::driver::Driver driver(clang_or_err.get(), llvm::sys::getDefaultTargetTriple(), diag_engine);

  auto *compilation = driver.BuildCompilation(args);
  SmallVector<std::pair<int, const Command *>, 0> failing_commands;
  if (compilation) { return driver.ExecuteCompilation(*compilation, failing_commands); }
  return 1;
}
