#include "libtanc.h"
#include "tanc.h"
#include <string>

#ifndef DEBUG
#define BEGIN_TRY                                                              \
try {

#else
#define BEGIN_TRY
#endif

#ifndef DEBUG
#define END_TRY                                                                \
  }                                                                            \
  catch (const std::exception &e) {                                            \
    std::cerr << "Error encountered in file " << app.current_filename()        \
              << ": " << e.what() << '\n';                                     \
    exit(1);                                                                   \
  }
#else
#define END_TRY
#endif

bool compile_file(const char *input_path, bool print_ast, bool print_ir_code) {
  std::string input_file(input_path);
  TanC<tanlang::Parser> app(std::vector<std::string>({input_file}), print_ast, print_ir_code);

  BEGIN_TRY

    bool r;
    r = app.read();
    if (!r) {
      return false;
    }
    r = app.parse();
    if (!r) {
      return false;
    }
    r = app.compile();
    if (!r) {
      return false;
    }

  END_TRY
  return true;
}

bool compile_files(unsigned n_files, char **input_paths, bool print_ast, bool print_ir_code) {
  std::vector<std::string> files;
  files.reserve(n_files);
  for (size_t i = 0; i < n_files; ++i) {
    files.push_back(std::string(input_paths[i]));
  }
  TanC<tanlang::Parser> app(files, print_ast, print_ir_code);
  bool r;
  while (!app.done()) {
    BEGIN_TRY

      r = app.read();
      if (!r) {
        return false;
      }
      r = app.parse();
      if (!r) {
        return false;
      }
      r = app.compile();
      if (!r) {
        return false;
      }
      app.next_file();

    END_TRY
  }
  return true;
}

extern bool evaluate_file(const char *input_path, bool print_ast, bool print_ir_code) {
  std::string input_file(input_path);
  TanC<tanlang::Interpreter> app(std::vector<std::string>({input_file}), print_ast, print_ir_code);
  BEGIN_TRY

    bool r;
    r = app.read();
    if (!r) {
      return false;
    }
    r = app.parse();
    if (!r) {
      return false;
    }
    r = app.compile();
    if (!r) {
      return false;
    }

  END_TRY
  return true;
}
