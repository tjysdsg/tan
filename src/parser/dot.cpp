#include "parser.h"
#include "src/ast/ast_dot.h"

namespace tanlang {
size_t ASTDot::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip "."
  _children.push_back(left); /// lhs
  auto member_name = parser->peek(_end_index);
  _end_index = member_name->nud(parser);
  if (member_name->_type == ASTType::ID || member_name->_type == ASTType::FUNC_CALL) {
    // TODO: check if member is in the struct
    // TODO: allow operator overriding?
    _children.push_back(member_name);
  } else {
    report_code_error(_token, "Invalid member access");
  }
  return _end_index;
}

} // namespace tanlang
