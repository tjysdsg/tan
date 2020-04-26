#include "src/ast/ast_prefix.h"
#include "parser.h"

namespace tanlang {

ASTPrefix::ASTPrefix(Token *token, size_t token_index) : ASTNode(ASTType::INVALID,
    PREC_LOWEST,
    0,
    token,
    token_index) {}

std::string ASTPrefix::get_type_name() const {
  assert(_children.size() > 0);
  return _children[0]->get_type_name();
}

llvm::Type *ASTPrefix::to_llvm_type(CompilerSession *compiler_session) const {
  assert(_children.size() > 0);
  return _children[0]->to_llvm_type(compiler_session);
}

llvm::Metadata *ASTPrefix::to_llvm_meta(CompilerSession *compiler_session) const {
  assert(_children.size() > 0);
  return _children[0]->to_llvm_meta(compiler_session);
}

std::shared_ptr<ASTTy> ASTPrefix::get_ty() const {
  assert(_children.size() > 0);
  return _children[0]->get_ty();
}

bool ASTPrefix::is_lvalue() const { return false; }

bool ASTPrefix::is_typed() const { return true; }

size_t ASTPrefix::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip self
  _children.push_back(parser->next_expression(_end_index, _lbp));
  return _end_index;
}

} // namespace tanlang
