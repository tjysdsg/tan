#include "src/ast/ast_ty.h"
#include "token.h"
#include "compiler_session.h"
#include "src/llvm_include.h"

namespace tanlang {

Value *ASTCharLiteral::_codegen(CompilerSession *cs) {
  _llvm_value = ConstantInt::get(cs->_builder->getInt8Ty(), (uint64_t) _c);
  return _llvm_value;
}

} // namespace tanlang
