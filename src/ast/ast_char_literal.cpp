#include "src/ast/ast_char_literal.h"
#include "src/ast/ast_ty.h"
#include "token.h"
#include "compiler_session.h"
#include "src/llvm_include.h"

namespace tanlang {

ASTCharLiteral::ASTCharLiteral(Token *t, size_t ti) : ASTLiteral(ASTType::CHAR_LITERAL,
    op_precedence[ASTType::CHAR_LITERAL],
    0,
    t,
    ti) { _c = t->value[0]; }

Value *ASTCharLiteral::_codegen(CompilerSession *cs) {
  _llvm_value = ConstantInt::get(cs->_builder->getInt8Ty(), (uint64_t) _c);
  return _llvm_value;
}

size_t ASTCharLiteral::nud() {
  _end_index = _start_index + 1; /// skip self
  _ty = ASTTy::Create(Ty::CHAR, vector<ASTNodePtr>());
  _ty->_default_value = static_cast<uint64_t>(_c);
  return _end_index;
}

} // namespace tanlang
