#include "ast.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"
#include <iostream>
#include "src/llvm_include.h"
using tanlang::Reader;
using tanlang::Parser;
using tanlang::Token;

int main() {
  std::string code =
      "fn work(hours: int, salary: float) : float { return hours * salary; } \n"
      "fn main() : float { return work(2, 2.1); } \n";
  Reader r;
  r.from_string(code);
  auto tokens = tokenize(&r);

  Parser p(tokens);
  p.parse();
  p._root->printTree();
  p.codegen();

  tanlang::Compiler compiler(std::shared_ptr<llvm::Module>(p.get_compiler_session()->get_module().release()));
  compiler.emit_object("output.o");

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
  return 0;
}
