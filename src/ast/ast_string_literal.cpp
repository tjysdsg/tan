#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

llvm::Value *ASTStringLiteral::get_llvm_value(CompilerSession *) const { return _llvm_value; }

Value *ASTStringLiteral::codegen(CompilerSession *compiler_session) {
  _llvm_type = compiler_session->get_builder()->getInt8PtrTy(); /// str as char*
  _llvm_value = compiler_session->get_builder()->CreateGlobalStringPtr(_svalue);
  return _llvm_value;
}

std::string ASTStringLiteral::get_type_name() const { return "str"; }

llvm::Type *ASTStringLiteral::to_llvm_type(CompilerSession *) const { return _llvm_type; }

std::shared_ptr<ASTTy> ASTStringLiteral::get_ty() const { return ASTTy::Create(TY_OR(Ty::STRING, Ty::POINTER)); }

ASTStringLiteral::ASTStringLiteral(Token *token, size_t token_index) : ASTLiteral(ASTType::STRING_LITERAL,
    op_precedence[ASTType::STRING_LITERAL],
    0,
    token,
    token_index) { _svalue = token->value; }

ASTStringLiteral::ASTStringLiteral(std::string str, size_t token_index) : ASTLiteral(ASTType::STRING_LITERAL,
    op_precedence[ASTType::STRING_LITERAL],
    0,
    nullptr,
    token_index) { _svalue = str; }

size_t ASTStringLiteral::nud() {
  _end_index = _start_index + 1; /// skip self
  return _end_index;
}

std::string ASTStringLiteral::get_string() const { return _svalue; }

} // namespace tanlang
