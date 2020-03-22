#include "parser.h"
#include "src/ast/ast_dot.h"

namespace tanlang {
void ASTDot::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.push_back(left);
  auto member_name = parser->advance();
  member_name->nud(parser);
  if (member_name->_type == ASTType::ID || member_name->_type == ASTType::FUNC_CALL) {
    // TODO: check if member is in the struct
    // TODO: allow operator overriding?
    _children.push_back(member_name);
  } else {
    report_code_error(_token, "Invalid member access");
  }
}
}
