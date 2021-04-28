#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>
#include <utility>

using namespace tanlang;

size_t ParserImpl::parse_member_access(const ASTNodePtr &left, const ASTNodePtr &p) {
  auto pma = ast_cast<ASTMemberAccess>(p);
  TAN_ASSERT(pma);
  if (at(p->_end_index)->value == "[") { pma->_access_type = MemberAccessType::MemberAccessBracket; }

  ++p->_end_index; /// skip "." or "["
  p->_children.push_back(left); /// lhs
  auto right = peek(p->_end_index);
  p->_end_index = parse_node(right);
  p->_children.push_back(right);

  if (pma->_access_type == MemberAccessType::MemberAccessBracket) {
    ++p->_end_index; /// skip ]
  } else if (pma->_access_type != MemberAccessType::MemberAccessBracket && right->get_token_str() == "*") {
    /// pointer dereference
    pma->_access_type = MemberAccessType::MemberAccessDeref;
    ++p->_end_index; /// skip *
  } else if (right->_type == ASTType::FUNC_CALL) { /// method call
    pma->_access_type = MemberAccessType::MemberAccessMemberFunction;
  }

  if (!(pma->_access_type == MemberAccessType::MemberAccessMemberFunction /// method call
      || pma->_access_type == MemberAccessType::MemberAccessDeref /// pointer dereference
      || right->_type == ASTType::ID /// member variable or enum
  )) {
    error(right->_end_index, "Invalid right-hand operand");
  }

  return p->_end_index;
}
