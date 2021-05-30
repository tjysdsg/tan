#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/expr.h"
#include "src/common.h"

using namespace tanlang;

size_t ParserImpl::parse_member_access(const ptr<Expr> &left, const ptr<MemberAccess> &p) {
  if (at(p->_end_index)->value == "[") {
    p->_access_type = MemberAccess::MemberAccessBracket;
  }

  ++p->_end_index; /// skip "." or "["

  /// lhs
  p->set_lhs(left);

  /// rhs
  auto _right = peek(p->_end_index);
  ptr<Expr> right = nullptr;
  if (!_right || !(right = ast_cast<Expr>(_right))) {
    error(p->_end_index, "Expect an expression");
  }
  p->_end_index = parse_node(right);
  p->set_rhs(right);

  if (p->_access_type == MemberAccess::MemberAccessBracket) { /// bracket access
    ++p->_end_index; /// skip ]
  } else if (p->_access_type != MemberAccess::MemberAccessBracket
      && right->get_token_str() == "*") { /// pointer dereference
    p->_access_type = MemberAccess::MemberAccessDeref;
    ++p->_end_index; // skip *
  } else if (right->get_node_type() == ASTNodeType::FUNC_CALL) { /// method call
    p->_access_type = MemberAccess::MemberAccessMemberFunction;
  }

  if (!(p->_access_type == MemberAccess::MemberAccessBracket
      || p->_access_type == MemberAccess::MemberAccessMemberFunction
      || p->_access_type == MemberAccess::MemberAccessDeref /// pointer dereference
      || right->get_node_type() == ASTNodeType::ID /// member variable or enum
  )) {
    error(right->_end_index, "Invalid right-hand operand");
  }

  return p->_end_index;
}
