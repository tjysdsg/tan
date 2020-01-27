#include "ast.h"
#include "reader.h"
#include "parser.h"
#include <iostream>

int main() {
  using tanlang::Reader;
  using tanlang::Parser;
  // std::string code = "if (arg0 * 2 < 0.5) { return (1 + 2) * 3 / 4; } else { return 1 + !arg1 < 5; }";
  std::string
      code =
      "fn main(weight: float, age: int) : float { return weight/age; } \n fn work(arg1 : float, arg2 : int) : float { return arg1 + arg2; }";
  Reader r;
  r.from_string(code);
  auto tokens = tokenize(&r);

  Parser p(tokens);
  p.parse();
  p._root->printTree();
  p._root->codegen(p._parser_context);
  p._parser_context->_module->print(llvm::errs(), nullptr);

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
  return 0;
}
