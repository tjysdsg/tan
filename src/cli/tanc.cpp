#include "ast.h"
#include "reader.h"
#include "parser.h"
#include <iostream>

int main() {
  using tanlang::Reader;
  using tanlang::Parser;
  std::string code =
      "fn main(weight: float, age: int) : float { var val : int = 100; val = 20.50; return val * weight/age; } \n";
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
