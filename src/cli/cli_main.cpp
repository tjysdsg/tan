#include "cli/cli.h"
#include "tan/tan.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <filesystem>
#include "config.h"
#include "driver/driver.h"

namespace cmd = llvm::cl;
namespace fs = std::filesystem;
using namespace tanlang;

int cli_main(int argc, char **argv) {
  // cmd parser
  cmd::OptionCategory cl_category("tanc");
  cmd::opt<str> opt_output_file("o", cmd::desc("Output filename"), cmd::value_desc("output"), cmd::init("a.out"),
                                cmd::cat(cl_category));
  cmd::list<str> opt_link_libraries("l", cmd::desc("Libraries to link against"), cmd::value_desc("libraries"),
                                    cmd::Prefix, cmd::cat(cl_category));
  cmd::list<str> opt_library_path("L", cmd::desc("Library search path"), cmd::Prefix, cmd::cat(cl_category));
  cmd::list<str> opt_source_files(cmd::Positional, cmd::Required, cmd::desc("Files to compile"),
                                  cmd::value_desc("<source files>"), cmd::OneOrMore, cmd::cat(cl_category));
  cmd::list<str> opt_import_dirs("I", cmd::desc("Import search directories"), cmd::Prefix, cmd::cat(cl_category));
  cmd::opt<bool> opt_print_ir_code("print-ir", cmd::desc("Print LLVM IR code"), cmd::cat(cl_category));
  cmd::opt<bool> opt_print_ast("print-ast", cmd::desc("Print abstract syntax tree"), cmd::cat(cl_category));
  cmd::opt<TanCompileType> opt_output_type(
      cmd::desc("Output type"),
      cmd::values(clEnumValN(DLIB, "shared", "Shared library"), clEnumValN(SLIB, "static", "Static library"),
                  clEnumValN(EXE, "exe", "Executable"), clEnumValN(OBJ, "obj", "Object file")),
      cmd::init(EXE), cmd::cat(cl_category));
  cmd::opt<TanOptLevel> opt_optimization_level(cmd::desc("Optimization level"),
                                               cmd::values(clEnumValN(O0, "g", "None"), clEnumVal(O0, "None"),
                                                           clEnumVal(O1, "Less"), clEnumVal(O2, "Default"),
                                                           clEnumVal(O3, "Aggressive")),
                                               cmd::init(O0), cmd::cat(cl_category));
  /// Remove options created by LLVM/Clang
  /// We don't want tons of flags not created by this file appearing in the output of `tanc --help`
  cmd::HideUnrelatedOptions(cl_category);
  cmd::ParseCommandLineOptions(
      argc, argv, fmt::format("tanc version: {}.{}.{} \n", TAN_VERSION[0], TAN_VERSION[1], TAN_VERSION[2]));

  // Init
  if (!init_compiler(argc, argv)) {
    // Cannot use Error class here since it's not initialized
    throw std::runtime_error("Unable to init tanc compiler");
  }

  try {
    vector<str> source_files;
    source_files.reserve(opt_source_files.size());
    /*
     * split by space
     * sometimes llvm::cl doesn't seem to split a string by space, causing opt_source_files containing an element
     * that should have been two elements
     */
    for (const auto &s : opt_source_files) {
      auto f = std::find(s.begin(), s.end(), ' ');
      if (f != s.end()) {
        source_files.emplace_back(s.begin(), f);
        source_files.emplace_back(f + 1, s.end());
      } else {
        source_files.push_back(s);
      }
    }

    // Build compilation config
    vector<str> lib_dirs;
    lib_dirs.reserve(opt_library_path.size());
    std::for_each(opt_library_path.begin(), opt_library_path.end(),
                  [&lib_dirs](const auto &s) { lib_dirs.push_back(s); });

    vector<str> link_files;
    link_files.reserve(opt_link_libraries.size());
    std::for_each(opt_link_libraries.begin(), opt_link_libraries.end(),
                  [&link_files](const auto &s) { link_files.push_back(s); });

    vector<str> import_dirs;
    import_dirs.reserve(opt_import_dirs.size());
    std::for_each(opt_import_dirs.begin(), opt_import_dirs.end(),
                  [&import_dirs](const auto &s) { import_dirs.push_back(s); });

    TanCompilation config;
    config.type = EXE;
    config.out_file = opt_output_file;
    config.lib_dirs = lib_dirs;
    config.link_files = link_files;
    config.import_dirs = import_dirs;
    config.type = opt_output_type.getValue();
    config.opt_level = opt_optimization_level.getValue();

    config.verbose = 0;
    if (opt_print_ast) {
      config.verbose = 2;
    } else if (opt_print_ir_code) {
      config.verbose = 1;
    }

    // Create and run CompilerDriver
    CompilerDriver driver(config);
    driver.run(source_files);

    return 0;

  } catch (const CompileException &e) {
    std::cerr << e.what() << '\n';
  }

  llvm::cl::ResetCommandLineParser();
  return 1;
}
