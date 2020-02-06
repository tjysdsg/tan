#include "ast.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"
#include <iostream>
#include "src/llvm_include.h"
using tanlang::Reader;
using tanlang::Parser;

int main() {
  std::string code =
      "fn main(weight: float, age: int) : float { var val : int = 100; val = 20.50; return val * weight/age; } \n";
  Reader r;
  r.from_string(code);
  auto tokens = tokenize(&r);

  tanlang::JIT jit(tokens);
  jit.parse();
  jit._root->printTree();
  jit.codegen();
  auto e = jit.evaluate();
  if (e) {
    throw std::runtime_error("JIT evaluation failed");
  }

  // tanlang::Compiler compiler(std::shared_ptr<llvm::Module>(p._parser_context->_module.release()));
  // compiler.emit_object("output.o");

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
  return 0;
}
