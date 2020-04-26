#include "src/ast/ast_infix_binary_op.h"
#include "parser.h"

namespace tanlang {

size_t ASTInfixBinaryOp::led(const ASTNodePtr &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip operator
  _children.emplace_back(left); /// lhs
  auto n = parser->next_expression(_end_index, _lbp);
  if (!n) {
    report_code_error(_token, "Unexpected token");
  } else {
    _children.emplace_back(n);
  }
  return _end_index;
}

ASTInfixBinaryOp::ASTInfixBinaryOp(Token *token, size_t token_index) : ASTNode(ASTType::INVALID,
    PREC_LOWEST,
    0,
    token,
    token_index) {}

std::string ASTInfixBinaryOp::get_type_name() const {
  assert(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->get_type_name();
}

std::shared_ptr<ASTTy> ASTInfixBinaryOp::get_ty() const {
  assert(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->get_ty();
}

llvm::Type *ASTInfixBinaryOp::to_llvm_type(CompilerSession *compiler_session) const {
  assert(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->to_llvm_type(compiler_session);
}

llvm::Metadata *ASTInfixBinaryOp::to_llvm_meta(CompilerSession *compiler_session) const {
  assert(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->to_llvm_meta(compiler_session);
}

bool ASTInfixBinaryOp::is_lvalue() const {
  return false;
}

bool ASTInfixBinaryOp::is_typed() const { return true; }

} // namespace tanlang
