#include "libtanc.h"
#include "compiler.h"
#include "lexer.h"
#include "linker.h"
#include "base.h"
#include "src/lib/llvm-ar.h"

#ifndef DEBUG
#define BEGIN_TRY try {
#else
#define BEGIN_TRY
#endif

#ifndef DEBUG
#define END_TRY                                                                \
  }                                                                            \
  catch (const std::exception &e) {                                            \
    std::cerr << "Error encountered in file " << files[i]                      \
              << ": " << e.what() << '\n';                                     \
    return false;                                                              \
  }
#else
#define END_TRY
#endif

static str opt_level_to_string(TanOptLevel l) {
  switch (l) {
    case O0:
      return "-O0";
    case O1:
      return "-O1";
    case O2:
      return "-O2";
    case O3:
      return "-O3";
    default:
      TAN_ASSERT(false);
  }
}

static bool _link(vector<str> input_paths, TanCompilation *config) {
  /// static
  if (config->type == SLIB) {
    vector<const char *> args = {"ar", "rcs", config->out_file};
    std::for_each(input_paths.begin(), input_paths.end(), [&args](const auto &s) { args.push_back(s.c_str()); });
    return !llvm_ar_main((int) args.size(), c_cast(char **, args.data()));
  }
  /// shared, obj, exe
  using tanlang::Linker;
  Linker linker;
  linker.add_files(input_paths);
  linker.add_flag("-o" + str(config->out_file));
  if (config->type == EXE) {
    /// default flags
    linker.add_flags({"-fPIE"});
  } else if (config->type == DLIB) {
    linker.add_flags({"-shared"});
  }
  /// -L
  for (size_t i = 0; i < config->n_lib_dirs; ++i) {
    auto p = fs::absolute(fs::path(config->lib_dirs[i]));
    linker.add_flag("-L" + p.string());
    linker.add_flag("-Wl,-rpath," + p.string());
  }
  /// -l
  for (size_t i = 0; i < config->n_link_files; ++i) {
    linker.add_flag("-l" + std::string(config->link_files[i]));
  }
  linker.add_flag(opt_level_to_string(config->opt_level));
  return linker.link();
}

bool compile_files(unsigned n_files, char **input_paths, TanCompilation *config) {
  bool print_ir_code = config->verbose >= 1;
  bool print_ast = config->verbose >= 2;

  /// input files
  vector<str> files;
  files.reserve(n_files);
  vector<str> obj_files;
  obj_files.reserve(n_files);
  for (size_t i = 0; i < n_files; ++i) {
    if (fs::path(input_paths[i]).extension() == ".tan") {
      files.push_back(str(input_paths[i]));
    } else if (fs::path(input_paths[i]).extension() == ".o") {
      obj_files.push_back(str(input_paths[i]));
    } else {
      std::cerr << "Unknown file extension: " << input_paths[i] << "\n";
      return false;
    }
  }
  /// config
  Compiler::compile_config = *config;
  /// import dirs
  Compiler::import_dirs.reserve(config->n_import_dirs);
  for (size_t i = 0; i < config->n_import_dirs; ++i) {
    Compiler::import_dirs.push_back(str(config->import_dirs[i]));
  }

  /// Compiler instances
  vector<std::shared_ptr<Compiler>> compilers{};
  compilers.reserve(n_files);

  /// parse all files before generating IR code
  for (size_t i = 0; i < n_files; ++i) {
    BEGIN_TRY
    auto compiler = std::make_shared<Compiler>(files[i]);
    compilers.push_back(compiler);
    compiler->parse();
    if (print_ast) { compiler->dump_ast(); }
    END_TRY
  }
  /// codegen
  for (size_t i = 0; i < n_files; ++i) {
    BEGIN_TRY
    compilers[i]->codegen();
    if (print_ir_code) { compilers[i]->dump_ir(); }
    /// prepare filename for linking
    files[i] += ".o";
    files[i] = fs::path(files[i]).filename().string();
    std::cout << "Compiling TAN file: " << files[i] << "\n";
    compilers[i]->emit_object(files[i]);
    END_TRY
  }

  /// link
  files.insert(files.begin(), obj_files.begin(), obj_files.end());
  if (config->type != OBJ) {
    bool ret = _link(files, config);
    if (!ret) { std::cerr << "Error linking files\n"; }
    return ret;
  } else {
    return true;
  }
}
