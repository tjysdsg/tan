#include "ast.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"
#include <iostream>
#include "src/llvm_include.h"
using tanlang::Reader;
using tanlang::Parser;
using tanlang::Token;
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
  jit.get_compiler_session()->get_module()->print(llvm::errs(), nullptr);
  auto e = jit.evaluate();
  if (e) {
    throw std::runtime_error("JIT evaluation failed");
  }

  auto main_func_symbol = jit.lookup("main");
  if (main_func_symbol.takeError()) {
    throw std::runtime_error("JIT symbol lookup failed");
  }

  auto *fp = (float (*)()) (intptr_t) main_func_symbol.get().getAddress();
  assert(fp && "Failed to codegen function");
  fprintf(stderr, "Evaluated to %f\n", fp());

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
  return 0;
}
