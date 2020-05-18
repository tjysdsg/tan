#include "src/ast/ast_infix_binary_op.h"
#include "src/type_system.h"
#include "src/ast/ast_ty.h"
#include "parser.h"

namespace tanlang {

size_t ASTInfixBinaryOp::led(const ASTNodePtr &left) {
  _end_index = _start_index + 1; /// skip operator
  _children.emplace_back(left); /// lhs
  auto n = _parser->next_expression(_end_index, _lbp);
  if (!n) { error(_end_index, "Unexpected token"); }
  else { _children.emplace_back(n); }
  _dominant_idx = this->get_dominant_idx();
  _ty = std::make_shared<ASTTy>(*_children[_dominant_idx]->get_ty());
  _ty->set_is_lvalue(false);
  return _end_index;
}

ASTInfixBinaryOp::ASTInfixBinaryOp(Token *t, size_t ti) : ASTNode(ASTType::INVALID, PREC_LOWEST, 0, t, ti) {}

llvm::Metadata *ASTInfixBinaryOp::to_llvm_meta(CompilerSession *cs) const {
  TAN_ASSERT(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->to_llvm_meta(cs);
}

bool ASTInfixBinaryOp::is_lvalue() const {
  return false;
}

bool ASTInfixBinaryOp::is_typed() const { return true; }

size_t ASTInfixBinaryOp::get_dominant_idx() const {
  auto lhs = _children[0];
  auto rhs = _children[1];
  TAN_ASSERT(lhs->is_typed());
  TAN_ASSERT(rhs->is_typed());
  int ret = TypeSystem::CanImplicitCast(lhs->get_ty(), rhs->get_ty());
  if (-1 == ret) {
    error("Cannot perform implicit type conversion");
  }
  return (size_t) ret;
}

} // namespace tanlang
