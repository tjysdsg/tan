#include "code_generator.h"
#include "code_generator_impl.h"
#include "src/ast/parsable_ast_node.h"

using namespace tanlang;

CodeGenerator::CodeGenerator(CompilerSession *cs) {
  _impl = new CodeGeneratorImpl(cs);
}

llvm::Value *CodeGenerator::codegen(CompilerSession *cs, const ParsableASTNodePtr &p) {
  return _impl->codegen(cs, p);
}

CodeGenerator::~CodeGenerator() {
  delete _impl;
}
