#include "cli.h"
#include "libtanc.h"
#include "base.h"
#include "config.h"
#include "src/llvm_include.h"

int cli_main(int argc, char **argv) {
  /// option parser
  cl::OptionCategory cl_category("tanc");
  cl::opt<std::string> opt_output_file
      ("o", cl::desc("Output filename"), cl::value_desc("output"), cl::init("a.out"), cl::cat(cl_category));
  cl::list<std::string> opt_link_libraries
      ("l", cl::desc("Libraries to link against"), cl::value_desc("libraries"), cl::Prefix, cl::cat(cl_category));
  cl::list<std::string> opt_source_files(cl::Positional,
      cl::Required,
      cl::desc("Files to compile"),
      cl::value_desc("<source files>"),
      cl::OneOrMore,
      cl::cat(cl_category));
  cl::list<std::string> opt_import_dirs("I", cl::desc("Import search directories"), cl::Prefix, cl::cat(cl_category));
  cl::opt<bool> opt_print_ir_code("print-ir", cl::desc("Print LLVM IR code"), cl::cat(cl_category));
  cl::opt<bool> opt_print_ast("print-ast", cl::desc("Print abstract syntax tree"), cl::cat(cl_category));
  cl::opt<TanCompileType> opt_output_type(cl::desc("Output type"),
      cl::values(clEnumValN(DLIB, "shared", "Shared library"),
          clEnumValN(SLIB, "static", "Static library"),
          clEnumValN(EXE, "exe", "Executable"),
          clEnumValN(OBJ, "obj", "Object file")),
      cl::init(EXE),
      cl::cat(cl_category));
  cl::opt<TanOptLevel> opt_optimization_level(cl::desc("Optimization level"),
      cl::values(clEnumValN(O0, "g", "None"),
          clEnumVal(O0, "None"),
          clEnumVal(O1, "Less"),
          clEnumVal(O2, "Default"),
          clEnumVal(O3, "Aggressive")),
      cl::init(O0),
      cl::cat(cl_category));
  auto &opt_map = cl::getRegisteredOptions();
  /// Remove options created by LLVM/Clang
  /// We don't want tons of flags not created by this file appearing in the output of `tanc --help`
  for (auto o: opt_map.keys()) {
    auto cats = opt_map[o]->Categories;
    bool found = false; /// search through categories, if no "tanc" option category, then hide it
    size_t n = cats.size();
    for (size_t i = 0; i < n; ++i) { /// since cats is llvm::SmallVector, we can just do a brute force search
      if (cats[i]->getName() == "tanc") { found = true; }
    }
    if (!found) { opt_map[o]->setHiddenFlag(cl::ReallyHidden); }
  }

  /// --version
  auto version_string = "v" + std::to_string(TAN_VERSION_MAJOR) + "." + std::to_string(TAN_VERSION_MINOR) + "."
      + std::to_string(TAN_VERSION_PATCH);
  cl::SetVersionPrinter([&version_string](llvm::raw_ostream &os) { os << "tanc " << version_string << "\n"; });

  cl::ParseCommandLineOptions(argc, argv, "tanc: compiler for TAN programming language\n\n"
                                          "tan, a fucking amazing programming language\n");

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

  /// import search dirs
  std::vector<char *> import_dirs;
  import_dirs.reserve(opt_import_dirs.size());
  std::for_each(opt_import_dirs.begin(),
      opt_import_dirs.end(),
      [&import_dirs](const std::string &s) { import_dirs.push_back(c_cast(char*, s.c_str())); });

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
  if (import_dirs.empty()) {
    config.import_dirs = nullptr;
    config.n_import_dirs = 0;
  } else {
    config.n_import_dirs = import_dirs.size();
    config.import_dirs = import_dirs.data();
  }

  /// output type
  config.type = opt_output_type.getValue();
  /// opt level
  config.opt_level = opt_optimization_level.getValue();

  /// verbosity
  if (opt_print_ast) { config.verbose = 2; }
  else if (opt_print_ir_code) { config.verbose = 1; }
  compile_files((unsigned) files.size(), files.data(), &config);
  return 0;
}

