#include "code_generator.h"
#include "code_generator_impl.h"
#include "src/ast/ast_base.h"

using namespace tanlang;

CodeGenerator::CodeGenerator(CompilerSession *cs) {
  _impl = new CodeGeneratorImpl(cs);
}

llvm::Value *CodeGenerator::codegen(const ASTBasePtr &p) {
  return _impl->codegen(ast_must_cast<ASTNode>(p));
}

CodeGenerator::~CodeGenerator() {
  delete _impl;
}