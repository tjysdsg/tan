#include "ast.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"
#include <iostream>
#include "src/llvm_include.h"
using tanlang::Reader;
using tanlang::Parser;
using tanlang::JIT;

int main() {
  std::string code =
      "fn work(hours: int, salary: float) : float { return hours * salary; } \n"
      "fn main() : float { return work(2, 2.1); } \n";
  Reader r;
  r.from_string(code);
  auto tokens = tokenize(&r);

  JIT jit(tokens);
  jit.parse();
  jit._root->printTree();
  jit.codegen();
  jit._parser_context->get_module()->print(llvm::errs(), nullptr);
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
