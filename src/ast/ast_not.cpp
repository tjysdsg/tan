#include "src/ast/ast_not.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

ASTNot::ASTNot(Token *token, size_t token_index) : ASTPrefix(token, token_index) {
  if (token->value == "!") { _type = ASTType::LNOT; }
  else if (token->value == "~") { _type = ASTType::BNOT; }
  else { TAN_ASSERT(false); }
  _lbp = op_precedence[_type];
}

Value *ASTNot::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  auto *rhs = _children[0]->codegen(compiler_session);
  if (_children[0]->is_lvalue()) {
    rhs = compiler_session->get_builder()->CreateLoad(rhs);
  }
  if (_type == ASTType::BNOT) {
    return compiler_session->get_builder()->CreateNot(rhs);
  }
  TAN_ASSERT(_type == ASTType::LNOT);
  /// get value size in bits
  auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
  if (rhs->getType()->isFloatingPointTy()) {
    return compiler_session->get_builder()
        ->CreateFCmpOEQ(rhs, ConstantFP::get(compiler_session->get_builder()->getFloatTy(), 0.0f));
  } else {
    return compiler_session->get_builder()
        ->CreateICmpEQ(rhs,
            ConstantInt::get(compiler_session->get_builder()->getIntNTy((unsigned) size_in_bits), 0, false));
  }
}

} // namespace tanlang
