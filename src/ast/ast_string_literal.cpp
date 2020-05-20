#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTStringLiteral::_codegen(CompilerSession *cs) {
  _llvm_value = cs->_builder->CreateGlobalStringPtr(_svalue);
  return _llvm_value;
}

ASTStringLiteral::ASTStringLiteral(Token *t, size_t ti) : ASTLiteral(ASTType::STRING_LITERAL,
    op_precedence[ASTType::STRING_LITERAL],
    0,
    t,
    ti), _svalue(t->value) {}

ASTStringLiteral::ASTStringLiteral(const str &s, size_t ti) : ASTLiteral(ASTType::STRING_LITERAL,
    op_precedence[ASTType::STRING_LITERAL],
    0,
    nullptr,
    ti), _svalue(s) {}

size_t ASTStringLiteral::nud() {
  _end_index = _start_index + 1; /// skip self
  _ty = ASTTy::Create(Ty::STRING);
  return _end_index;
}

str ASTStringLiteral::get_string() { return _svalue; }

} // namespace tanlang
