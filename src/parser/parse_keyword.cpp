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

size_t ParserImpl::parse_intrinsic(const ASTNodePtr &p) {
  ++p->_end_index; /// skip "@"
  auto e = peek(p->_end_index);
  p->_end_index = parse_node(e);
  /// Only allow identifier or function call as valid intrinsic token
  if (e->_type != ASTType::ID && e->_type != ASTType::FUNC_CALL) {
    error(e->_end_index, "Unexpected token");
  }
  p->_children.push_back(e);
  return p->_end_index;
}

size_t ParserImpl::parse_import(const ASTNodePtr &p) {
  ++p->_end_index; /// skip "import"
  auto rhs = peek(p->_end_index);
  if (rhs->_type != ASTType::STRING_LITERAL) {
    error(p->_end_index, "Invalid import statement");
  }
  p->_end_index = parse_node(rhs);
  str filename = std::get<str>(rhs->_value);
  p->_name = filename;
  return p->_end_index;
}
