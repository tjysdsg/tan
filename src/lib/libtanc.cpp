#include "libtanc.h"
#include "compiler.h"
#include "lexer.h"
#include "linker.h"
#include "parser.h"
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

static bool _link(std::vector<std::string> input_paths, TanCompilation *config) {
  using tanlang::Linker;
  Linker linker;
  linker.add_flag("-o" + std::string(config->out_file));
  linker.add_files(input_paths);
  /// default flags
  linker.add_flags({"-fPIE"});
  return linker.link();
}

bool compile_files(unsigned n_files, char **input_paths, TanCompilation *config) {
  bool print_ir_code = config->verbose >= 1;
  bool print_ast = config->verbose >= 2;
  std::vector<std::string> files;
  files.reserve(n_files);
  for (size_t i = 0; i < n_files; ++i) {
    files.push_back(std::string(input_paths[i]));
  }
  for (size_t i = 0; i < n_files; ++i) {
    BEGIN_TRY

    tanlang::Reader reader;
    reader.open(files[i]);
    auto tokens = tanlang::tokenize(&reader);
    tanlang::Parser parser(tokens, files[i]);
    parser.parse();
    if (print_ast) { parser._root->printTree(); }
    std::cout << "Compiling TAN file: " << files[i] << "\n";
    parser.codegen();
    tanlang::Compiler compiler(parser.get_compiler_session(), config);
    if (print_ir_code) { compiler.dump(); }
    /// prepare the filename for linking
    files[i] += ".o";
    files[i] = std::filesystem::path(files[i]).filename().string();
    compiler.emit_object(files[i]);

    END_TRY
  }

  for (size_t i = 0; i < config->n_link_files; ++i) {
    files.push_back(std::string(config->link_files[i]));
  }
  if (config->type == EXE) {
    _link(files, config);
  } else if (config->type == OBJ) {
  } else {
    // TODO
    assert(false);
  }
  return true;
}

bool tan_link(unsigned n_files, char **input_paths, TanCompilation *config) {
  std::vector<std::string> files;
  files.reserve(n_files + config->n_link_files);
  for (size_t i = 0; i < n_files; ++i) {
    files.push_back(std::string(input_paths[i]));
  }
  for (size_t i = 0; i < config->n_link_files; ++i) {
    files.push_back(std::string(config->link_files[i]));
  }
  return _link(files, config);
}
