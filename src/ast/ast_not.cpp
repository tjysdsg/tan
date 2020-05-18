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

Value *ASTNot::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  auto *rhs = _children[0]->codegen(cs);
  if (!rhs) { error("Invalid operand"); }
  if (_children[0]->is_lvalue()) { rhs = builder->CreateLoad(rhs); }
  if (_type == ASTType::BNOT) {
    _llvm_value = builder->CreateNot(rhs);
  } else if (_type == ASTType::LNOT) {
    /// get value size in bits
    auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
    if (rhs->getType()->isFloatingPointTy()) {
      _llvm_value = builder->CreateFCmpOEQ(rhs, ConstantFP::get(builder->getFloatTy(), 0.0f));
    } else {
      _llvm_value = builder->CreateICmpEQ(rhs, ConstantInt::get(builder->getIntNTy((unsigned) size_in_bits), 0, false));
    }
  } else { TAN_ASSERT(false); }
  return _llvm_value;
}

size_t ASTNot::nud() {
  auto ret = ASTPrefix::nud();
  if (_type == ASTType::LNOT) { _ty = ASTTy::Create(Ty::BOOL, vector<ASTNodePtr>()); }
  return ret;
}

} // namespace tanlang
