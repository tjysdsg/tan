#include "tanc.h"
#include "src/lib/libtanc.h"
#include <string>

bool compile_file(const char *input_path, bool print_ast, bool print_ir_code) {
  std::string input_file(input_path);
  TanC<tanlang::Parser> app(std::vector<std::string>({input_file}), print_ast, print_ir_code);
  bool r;
  #ifndef DEBUG
  try {
  #endif
  r = app.read();
  if (!r) { return false; }
  r = app.parse();
  if (!r) { return false; }
  r = app.compile();
  if (!r) { return false; }
  #ifndef DEBUG
  } catch (const std::exception &e) {
    std::cerr << "Error encountered while evaluating file " << input_file << ": " << e.what() << '\n';
    exit(1);
  }
  #endif
  return true;
}

extern bool evaluate_file(const char *input_path, bool print_ast, bool print_ir_code) {
  std::string input_file(input_path);
  TanC<tanlang::Interpreter> app(std::vector<std::string>({input_file}), print_ast, print_ir_code);
  #ifndef DEBUG
  try {
  #endif
  bool r;
  r = app.read();
  if (!r) { return false; }
  r = app.parse();
  if (!r) { return false; }
  r = app.compile();
  if (!r) { return false; }
  #ifndef DEBUG
  } catch (const std::exception &e) {
    std::cerr << "Error encountered while evaluating file " << input_file << ": " << e.what() << '\n';
    exit(1);
  }
  #endif
  return true;
}
