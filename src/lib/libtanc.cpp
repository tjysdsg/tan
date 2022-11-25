#include "libtanc.h"
#include "compiler.h"
#include "lexer.h"
#include "linker.h"
#include "src/lib/misc.h"
#include "src/lib/llvm-ar.h"
#include "backtrace/tan_backtrace.h"
#include <filesystem>

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

namespace fs = std::filesystem;

static str search_library(const vector<str> &lib_dirs, const str &lib_name) {
  for (const str &dir: lib_dirs) {
    vector<fs::path> candidates = { /// possible filenames
        fs::path(dir) / fs::path(lib_name),                 //
        fs::path(dir) / fs::path(lib_name + ".a"),          //
        fs::path(dir) / fs::path(lib_name + ".so"),         //
        fs::path(dir) / fs::path("lib" + lib_name + ".a"),  //
        fs::path(dir) / fs::path("lib" + lib_name + ".so"), //
    };
    for (const auto &p: candidates) {
      if (fs::exists(p)) {
        return p.string();
      }
    }
  }
  return "";
}

static bool _link(vector<str> input_paths, TanCompilation *config) {
  /// static
  if (config->type == SLIB) {
    /// also add files specified by -l option
    for (const auto &lib: config->link_files) {
      str path = search_library(config->lib_dirs, lib);

      if (path.empty()) {
        std::cerr << fmt::format("Unable to find library: {}\n", lib);
        return false;
      }
      input_paths.push_back(path);
    }
    return !llvm_ar_create_static_lib(config->out_file, input_paths);
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
  size_t n_lib_dirs = config->lib_dirs.size();
  for (size_t i = 0; i < n_lib_dirs; ++i) {
    auto p = fs::absolute(fs::path(config->lib_dirs[i]));
    linker.add_flag("-L" + p.string());
    linker.add_flag("-Wl,-rpath," + p.string());
  }
  /// -l
  size_t n_link_files = config->link_files.size();
  for (size_t i = 0; i < n_link_files; ++i) {
    linker.add_flag("-l" + std::string(config->link_files[i]));
  }
  linker.add_flag(opt_level_to_string(config->opt_level));
  return linker.link();
}

bool compile_files(vector<str> input_paths, TanCompilation *config) {
  bool print_ir_code = config->verbose >= 1;
  bool print_ast = config->verbose >= 2;

  /// input files
  size_t n_files = input_paths.size();
  vector<str> files;
  files.reserve(n_files);
  vector<str> obj_files;
  obj_files.reserve(n_files);
  for (size_t i = 0; i < n_files; ++i) {
    if (fs::path(input_paths[i]).extension() == ".tan") {
      files.push_back(input_paths[i]);
    } else if (fs::path(input_paths[i]).extension() == ".o") {
      obj_files.push_back(input_paths[i]);
    } else {
      std::cerr << "Unknown file extension: " << input_paths[i] << "\n";
      return false;
    }
  }
  n_files = files.size();
  /// config
  Compiler::compile_config = *config;
  /// import dirs
  size_t n_import = config->import_dirs.size();
  Compiler::import_dirs.reserve(n_import);
  Compiler::import_dirs.insert(Compiler::import_dirs.begin(), config->import_dirs.begin(), config->import_dirs.end());

  /// Compiler instances
  vector<Compiler *> compilers{};
  compilers.reserve(n_files);

  /// parse all files before generating IR code
  for (size_t i = 0; i < n_files; ++i) {
    BEGIN_TRY
    auto compiler = new Compiler(files[i]);
    compilers.push_back(compiler);
    compiler->parse();
    if (print_ast) { compiler->dump_ast(); }
    END_TRY
  }
  /// _codegen
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

bool init_compiler(int argc, char **argv) {
  return init_back_trace(argv[0]);
}
