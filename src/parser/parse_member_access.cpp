#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_member_access.h"
#include "src/common.h"

using namespace tanlang;

size_t ParserImpl::parse_member_access(const ParsableASTNodePtr &left, const ParsableASTNodePtr &p) {
  auto pma = ast_cast<ASTMemberAccess>(p);
  TAN_ASSERT(pma);
  if (at(p->_end_index)->value == "[") { pma->_access_type = MemberAccessType::MemberAccessBracket; }

  ++p->_end_index; /// skip "." or "["
  p->append_child(left); /// lhs
  auto right = peek(p->_end_index);
  p->_end_index = parse_node(right);
  p->append_child(right);

  if (pma->_access_type == MemberAccessType::MemberAccessBracket) {
    ++p->_end_index; /// skip ]
  } else if (pma->_access_type != MemberAccessType::MemberAccessBracket && right->get_token_str() == "*") {
    /// pointer dereference
    pma->_access_type = MemberAccessType::MemberAccessDeref;
    ++p->_end_index; /// skip *
  } else if (right->get_node_type() == ASTType::FUNC_CALL) { /// method call
    pma->_access_type = MemberAccessType::MemberAccessMemberFunction;
  }

  if (!(pma->_access_type == MemberAccessType::MemberAccessMemberFunction /// method call
      || pma->_access_type == MemberAccessType::MemberAccessDeref /// pointer dereference
      || right->get_node_type() == ASTType::ID /// member variable or enum
  )) {
    error(right->_end_index, "Invalid right-hand operand");
  }

  return p->_end_index;
}
