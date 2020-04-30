#include "src/ast/ast_char_literal.h"
#include "src/ast/ast_ty.h"
#include "token.h"
#include "compiler_session.h"
#include "src/llvm_include.h"

namespace tanlang {

ASTCharLiteral::ASTCharLiteral(Token *token, size_t token_index) : ASTLiteral(ASTType::CHAR_LITERAL,
    op_precedence[ASTType::CHAR_LITERAL],
    0,
    token,
    token_index) { _c = token->value[0]; }

llvm::Value *ASTCharLiteral::get_llvm_value(CompilerSession *) const { return _llvm_value; }

Value *ASTCharLiteral::codegen(CompilerSession *cs) {
  _llvm_type = cs->get_builder()->getInt8Ty(); /// char = u8
  _llvm_value = ConstantInt::get(cs->get_builder()->getInt8Ty(), (uint64_t) _c);
  return _llvm_value;
}

std::string ASTCharLiteral::get_type_name() const { return "char"; }

llvm::Type *ASTCharLiteral::to_llvm_type(CompilerSession *) const { return _llvm_type; }

std::shared_ptr<ASTTy> ASTCharLiteral::get_ty() const {
  // TODO: optimize this
  return ASTTy::Create(Ty::CHAR);
}

size_t ASTCharLiteral::nud() {
  _end_index = _start_index + 1; /// skip self
  return _end_index;
}

} // namespace tanlang
