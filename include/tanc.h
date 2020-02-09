#ifndef TAN_INCLUDE_TANC_H_
#define TAN_INCLUDE_TANC_H_
#include "token.h"
#include "parser.h"
#include "compiler.h"
using tanlang::Token;
using tanlang::Compiler;

template<typename PARSER_TYPE>
class TanC final {
 public:
  TanC() = delete;
  TanC(std::vector<std::string> files, bool print_ast, bool print_ir_code);
  ~TanC();
  bool read();
  bool parse();
  bool compile();
  void next_file() { ++_curr_file; }

 private:
  std::vector<std::string> _input_files{};
  bool _print_ast = false;
  bool _print_ir_code = false;
  std::vector<Token *> _tokens{};
  std::unique_ptr<PARSER_TYPE> _parser{};
  std::unique_ptr<Compiler> _compiler{};
  size_t _curr_file = 0;
};
#include "src/tanc/tanc.hpp"

#endif /* TAN_INCLUDE_TANC_H_ */
