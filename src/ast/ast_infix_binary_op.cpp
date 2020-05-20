#include "src/ast/ast_infix_binary_op.h"
#include "src/ast/ast_ty.h"
#include "parser.h"

namespace tanlang {

llvm::Metadata *ASTInfixBinaryOp::to_llvm_meta(CompilerSession *cs) {
  TAN_ASSERT(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->to_llvm_meta(cs);
}

size_t ASTInfixBinaryOp::get_dominant_idx() {
  auto lhs = _children[0];
  auto rhs = _children[1];
  if (!lhs->is_typed()) { error("Invalid left-hand operand"); }
  if (!rhs->is_typed()) { error("Invalid right-hand operand"); }
  int ret = TypeSystem::CanImplicitCast(lhs->get_ty(), rhs->get_ty());
  if (-1 == ret) { error("Cannot perform implicit type conversion"); }
  return (size_t) ret;
}

} // namespace tanlang
