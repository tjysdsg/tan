#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/parsable_ast_node.h"

using namespace tanlang;

size_t ParserImpl::parse_intrinsic(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip "@"
  auto e = peek(p->_end_index);
  p->_end_index = parse_node(e);
  /// Only allow identifier or function call as valid intrinsic token
  if (e->get_node_type() != ASTType::ID && e->get_node_type() != ASTType::FUNC_CALL) {
    error(e->_end_index, "Unexpected token");
  }
  p->append_child(e);
  return p->_end_index;
}

size_t ParserImpl::parse_import(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip "import"
  auto rhs = peek(p->_end_index);
  if (rhs->get_node_type() != ASTType::STRING_LITERAL) {
    error(p->_end_index, "Invalid import statement");
  }
  p->_end_index = parse_node(rhs);
  str filename = rhs->get_data<str>();
  p->set_data(filename);
  return p->_end_index;
}
