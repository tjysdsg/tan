#include "libtanc.h"
#include "compiler.h"
#include "lexer.h"
#include "linker.h"
#include "parser.h"
#include "reader.h"
#include <string>

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
    BEGIN_TRY ;
      tanlang::Reader reader;
      reader.open(files[i]);
      auto tokens = tanlang::tokenize(&reader);
      tanlang::Parser parser(tokens, files[i]);
      parser.parse();
      if (print_ast) { parser._root->printTree(); }
      std::cout << "Compiling TAN file: " << files[i] << "\n";
      parser.codegen();
      if (print_ir_code) { parser.dump(); }
      tanlang::Compiler compiler(parser.get_compiler_session()->get_module().release(), config);
      files[i] += ".o"; /// prepare the filename for linking
      compiler.emit_object(files[i]); END_TRY;
  }

  _link(files, config);
  return true;
}

bool tan_link(unsigned n_files, char **input_paths, TanCompilation *config) {
  if (config->type == OBJ) { return true; }
  std::vector<std::string> files;
  files.reserve(n_files);
  for (size_t i = 0; i < n_files; ++i) {
    files.push_back(std::string(input_paths[i]));
  }
  return _link(files, config);
}
