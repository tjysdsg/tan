#ifndef TAN_SRC_CLI_APP_H_
#define TAN_SRC_CLI_APP_H_
#include "token.h"
#include "parser.h"
#include "compiler.h"
using tanlang::Token;
using tanlang::Compiler;

template<typename PARSER_TYPE>
class App final {
 public:
  App() = delete;
  App(int argc, char **argv);
  ~App();
  bool read();
  bool parse();
  bool compile();
  void next_file() { ++_curr_file; }

 private:
  std::vector<std::string> _input_files{};
  std::string _output_file{};
  bool _print_ast = false;
  bool _print_ir_code = false;
  std::vector<Token *> _tokens{};
  std::unique_ptr<PARSER_TYPE> _parser{};
  std::unique_ptr<Compiler> _compiler{};
  size_t _curr_file = 0;
};
#include "src/cli/App.hpp"

#endif // TAN_SRC_CLI_APP_H_
