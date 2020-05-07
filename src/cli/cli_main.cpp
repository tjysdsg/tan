#include "cli.h"
#include "libtanc.h"
#include "base.h"
#include <iostream>
#include <llvm/Support/CommandLine.h>

namespace cl = llvm::cl;
static cl::opt<std::string>
    opt_output_file("o", cl::desc("Output filename"), cl::value_desc("output"), cl::init("a.out"));
static cl::list<std::string>
    opt_link_libraries("l", cl::desc("Libraries to link against"), cl::value_desc("libraries"), cl::Prefix);
static cl::list<std::string> opt_source_files
    (cl::Positional, cl::Required, cl::desc("Files to compile"), cl::value_desc("<source files>"), cl::OneOrMore);
static cl::opt<bool> opt_print_ir_code("print-ir", cl::desc("Print LLVM IR code"));
static cl::opt<bool> opt_print_ast("print-ast", cl::desc("Print abstract syntax tree"));
// static cl::opt<bool> opt_version("version", cl::desc("Print tanc version and exit"));
static cl::opt<TanCompileType> opt_output_type(cl::desc("Output type"),
    cl::values(clEnumValN(DLIB, "shared", "Shared library"),
        clEnumValN(SLIB, "static", "Static library"),
        clEnumValN(EXE, "exe", "Executable"),
        clEnumValN(OBJ, "obj", "Object file")),
    cl::init(EXE));

int cli_main(int *pargc, char ***pargv) {
  cl::ParseCommandLineOptions(*pargc, *pargv, "tanc: compiler for TAN programming language\n\n"
                                              "tan, a fucking amazing programming language\n\n");

  auto version_string = std::to_string(TAN_VERSION_MAJOR) + "." + std::to_string(TAN_VERSION_MINOR) + "."
      + std::to_string(TAN_VERSION_PATCH);

  /// --version
  // if (opt_version.getValue()) { std::cout << version_string << "\n"; }

  /// source files
  std::vector<char *> files;
  files.reserve(opt_source_files.size());
  std::for_each(opt_source_files.begin(),
      opt_source_files.end(),
      [&files](const std::string &s) { files.push_back(c_cast(char*, s.c_str())); });

  /// files to link to
  std::vector<char *> link_files;
  link_files.reserve(opt_link_libraries.size());
  std::for_each(opt_link_libraries.begin(),
      opt_link_libraries.end(),
      [&link_files](const std::string &s) { link_files.push_back(c_cast(char*, s.c_str())); });

  /// build config
  TanCompilation config;
  config.type = EXE;
  config.out_file = opt_output_file.c_str();
  config.verbose = 0;
  if (link_files.empty()) {
    config.link_files = nullptr;
    config.n_link_files = 0;
  } else {
    config.n_link_files = link_files.size();
    config.link_files = link_files.data();
  }

  /// output type
  config.type = opt_output_type.getValue();

  /// verbosity
  if (opt_print_ast) { config.verbose = 2; }
  else if (opt_print_ir_code) { config.verbose = 1; }
  compile_files((unsigned) files.size(), files.data(), &config);
  return 0;
}

