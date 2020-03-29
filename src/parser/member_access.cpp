#include "parser.h"
#include "src/ast/ast_member_access.h"

namespace tanlang {
size_t ASTMemberAccess::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip "." or "["
  _is_bracket = parser->at(_start_index)->value == "[";
  _children.push_back(left); /// lhs
  auto member_name = parser->peek(_end_index);
  _end_index = member_name->parse(parser);
  if (member_name->_type == ASTType::ID || member_name->_type == ASTType::FUNC_CALL
      || member_name->_type == ASTType::NUM_LITERAL) {
    // TODO: check if member is in the struct
    // TODO: check whether array element access is in bound
    // TODO: allow operator overriding?
    _children.push_back(member_name);
  } else {
    report_code_error(_token, "Invalid member access");
  }

  if (_is_bracket) { ++_end_index; } /// skip "]" if this is a bracket access
  return _end_index;
}

} // namespace tanlang
