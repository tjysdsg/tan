#include "cli/cli.h"
#include "tan/tan.h"
#include "llvm_api.h"
#include "llvm/Support/CommandLine.h"
#include <iostream>
#include <filesystem>

namespace cmd = llvm::cl;
namespace fs = std::filesystem;
using namespace tanlang;

/// \see https://gcc.gnu.org/onlinedocs/gcc-4.4.1/gcc/Overall-Options.html
static constexpr std::array cxx_ext{"cpp", "CPP", "cxx", "c",  "cc",  "C",   "c++", "cp",  "i",  "ii",
                                    "h",   "hh",  "H",   "hp", "hxx", "hpp", "HPP", "h++", "tcc"};

int cli_main(int argc, char **argv) {
  llvm::cl::ResetCommandLineParser();

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
  cmd::ParseCommandLineOptions(argc, argv,
                               "tanc: compiler for TAN programming language\n\n"
                               "tan, a fucking amazing programming language\n");

  /// init
  if (!init_compiler(argc, argv)) {
    std::cerr << "Unable to init tanc compiler\n";
    abort();
  }

  /// tan source files
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

  vector<str> tan_files;
  tan_files.reserve(source_files.size());
  /// cxx
  vector<str> cxx_files;
  cxx_files.reserve(source_files.size());
  for (size_t i = 0; i < source_files.size(); ++i) {
    bool is_cxx = false;
    std::for_each(cxx_ext.begin(), cxx_ext.end(), [&is_cxx, &source_files, i](const str &s) {
      auto n = s.length();
      if (n <= source_files[i].length()) {
        is_cxx |= std::equal(source_files[i].rbegin(), source_files[i].rbegin() + (long)n, s.rbegin(), s.rend());
      }
    });
    if (is_cxx) {
      cxx_files.push_back(source_files[i]);
    } /// cxx files
    else {
      tan_files.push_back(source_files[i]);
    } /// tan files
  }

  /// lib dirs
  vector<str> lib_dirs;
  lib_dirs.reserve(opt_library_path.size());
  std::for_each(opt_library_path.begin(), opt_library_path.end(),
                [&lib_dirs](const auto &s) { lib_dirs.push_back(s); });

  /// files to link to
  vector<str> link_files;
  link_files.reserve(opt_link_libraries.size());
  std::for_each(opt_link_libraries.begin(), opt_link_libraries.end(),
                [&link_files](const auto &s) { link_files.push_back(s); });

  /// import search dirs
  vector<str> import_dirs;
  import_dirs.reserve(opt_import_dirs.size());
  std::for_each(opt_import_dirs.begin(), opt_import_dirs.end(),
                [&import_dirs](const auto &s) { import_dirs.push_back(s); });

  /// build config
  TanCompilation config;
  config.type = EXE;
  config.out_file = opt_output_file;
  config.verbose = 0;
  config.lib_dirs = lib_dirs;
  config.link_files = link_files;
  config.import_dirs = import_dirs;

  /// output type
  config.type = opt_output_type.getValue();
  /// opt level
  config.opt_level = opt_optimization_level.getValue();

  /// verbosity
  if (opt_print_ast) {
    config.verbose = 2;
  } else if (opt_print_ir_code) {
    config.verbose = 1;
  }

  /// compile cxx files first
  if (!cxx_files.empty()) {
    std::cout << "Compiling " << cxx_files.size() << " CXX file(s): ";
    std::for_each(cxx_files.begin(), cxx_files.end(), [=](auto f) { std::cout << f << " "; });
    std::cout << "\n";
    auto err_code = clang_compile(cxx_files, &config);
    if (err_code) {
      return err_code;
    }
    /// add cxx object files
    size_t n = cxx_files.size();
    vector<str> obj_files{};
    obj_files.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      auto p = fs::path(str(cxx_files[i])).replace_extension(".o").filename();
      obj_files.push_back(p.string());
    }
    tan_files.insert(tan_files.end(), obj_files.begin(), obj_files.end());
  }

  return !compile_files(tan_files, &config);
}
