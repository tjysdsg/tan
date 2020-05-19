#include "src/ast/ast_prefix.h"
#include "parser.h"

namespace tanlang {

ASTPrefix::ASTPrefix(Token *t, size_t ti) : ASTNode(ASTType::INVALID, 0, 0, t, ti) {}

bool ASTPrefix::is_typed() { return true; }

size_t ASTPrefix::nud() {
  _end_index = _start_index + 1; /// skip self
  auto rhs = _parser->next_expression(_end_index, _lbp);
  _ty = rhs->get_ty();
  _children.push_back(rhs);
  return _end_index;
}

} // namespace tanlang
