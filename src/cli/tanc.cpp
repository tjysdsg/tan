#include "ast.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"
#include <iostream>
#include "src/llvm_include.h"
using tanlang::Reader;
using tanlang::Parser;

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // tanlang::JIT *jit = tanlang::JIT::Create();
  std::string code =
      "fn main(weight: float, age: int) : float { var val : int = 100; val = 20.50; return val * weight/age; } \n";
  Reader r;
  r.from_string(code);
  auto tokens = tokenize(&r);

  // Parser p(tokens, jit);
  Parser p(tokens);
  p.parse();
  p._root->printTree();
  p._root->codegen(p._parser_context);
  p._parser_context->get_module()->print(llvm::errs(), nullptr);

  /*
  // Get the anonymous expression's JITSymbol.
  auto Sym = jit->lookup(("__anon_expr" + llvm::Twine(1)).str());

  auto *FP = (double (*)()) (intptr_t) Sym->getAddress();
  assert(FP && "Failed to codegen function");
  fprintf(stderr, "Evaluated to %f\n", FP());
   */

  // tanlang::Compiler compiler(std::shared_ptr<llvm::Module>(p._parser_context->_module.release()));
  // compiler.emit_object("output.o");

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
  // delete jit;
  return 0;
}
