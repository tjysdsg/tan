#include "src/ast/ast_not.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTBinaryNot::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  auto *rhs = _children[0]->codegen(compiler_session);
  if (_children[0]->is_lvalue()) {
    rhs = compiler_session->get_builder()->CreateLoad(rhs);
  }
  return compiler_session->get_builder()->CreateNot(rhs);
}

Value *ASTLogicalNot::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  auto *rhs = _children[0]->codegen(compiler_session);
  if (_children[0]->is_lvalue()) {
    rhs = compiler_session->get_builder()->CreateLoad(rhs);
  }
  auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
  /// get value size in bits
  if (rhs->getType()->isFloatingPointTy()) {
    return compiler_session->get_builder()
        ->CreateFCmpOEQ(rhs, ConstantFP::get(compiler_session->get_builder()->getFloatTy(), 0.0f));
  } else {
    return compiler_session->get_builder()
        ->CreateICmpEQ(rhs,
            ConstantInt::get(compiler_session->get_builder()->getIntNTy((unsigned) size_in_bits), 0, false));
  }
}

ASTLogicalNot::ASTLogicalNot(Token *token, size_t token_index) : ASTPrefix(token, token_index) {
  _type = ASTType::LNOT;
  _lbp = op_precedence[_type];
}

ASTBinaryNot::ASTBinaryNot(Token *token, size_t token_index) : ASTPrefix(token, token_index) {
  _type = ASTType::BNOT;
  _lbp = op_precedence[_type];
}

} // namespace tanlang
