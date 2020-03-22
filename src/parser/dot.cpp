#include "parser.h"
#include "src/ast/ast_dot.h"

namespace tanlang {
void ASTDot::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.push_back(left);
  auto member_name = parser->peek();
  if (member_name->_type == ASTType::ID) {
    // TODO: check if member is in the struct
    // TODO: allow operator overriding?
    ++parser->_curr_token;
    _children.push_back(member_name);
  } else {
    report_code_error(_token, "Invalid member access");
  }
}
}
