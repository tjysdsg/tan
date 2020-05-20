#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTStringLiteral::_codegen(CompilerSession *cs) {
  _llvm_value = cs->_builder->CreateGlobalStringPtr(_svalue);
  return _llvm_value;
}

} // namespace tanlang
