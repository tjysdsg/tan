#include "src/ast/ast_not.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

ASTNot::ASTNot(Token *token, size_t token_index) : ASTPrefix(token, token_index) {
  if (token->value == "!") { _type = ASTType::LNOT; }
  else if (token->value == "~") { _type = ASTType::BNOT; }
  else { TAN_ASSERT(false); }
  _lbp = op_precedence[_type];
}

Value *ASTNot::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  auto *rhs = _children[0]->codegen(cs);
  if (_children[0]->is_lvalue()) { rhs = cs->get_builder()->CreateLoad(rhs); }
  if (_type == ASTType::BNOT) {
    _llvm_value = cs->get_builder()->CreateNot(rhs);
  } else if (_type == ASTType::LNOT) {
    /// get value size in bits
    auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
    if (rhs->getType()->isFloatingPointTy()) {
      _llvm_value = cs->get_builder()->CreateFCmpOEQ(rhs, ConstantFP::get(cs->get_builder()->getFloatTy(), 0.0f));
    } else {
      _llvm_value = cs->get_builder()
          ->CreateICmpEQ(rhs, ConstantInt::get(cs->get_builder()->getIntNTy((unsigned) size_in_bits), 0, false));
    }
  } else { TAN_ASSERT(false); }
  return _llvm_value;
}

size_t ASTNot::nud() {
  auto ret = ASTPrefix::nud();
  if (_type == ASTType::LNOT) { _ty = ASTTy::Create(Ty::BOOL); }
  return ret;
}

} // namespace tanlang
