#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_base.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/intrinsic.h"

using namespace tanlang;

size_t ParserImpl::parse_intrinsic(const ASTBasePtr &_p) {
  ptr<Intrinsic> p = ast_must_cast<Intrinsic>(_p);

  ++p->_end_index; /// skip "@"
  auto e = peek(p->_end_index);
  p->_end_index = parse_node(e);
  /// Only allow identifier or function call as valid intrinsic token
  if (e->get_node_type() != ASTNodeType::ID && e->get_node_type() != ASTNodeType::FUNC_CALL) {
    error(e->_end_index, "Unexpected token");
  }
  p->set_sub(ast_must_cast<ASTNamed>(e));
  return p->_end_index;
}

size_t ParserImpl::parse_import(const ASTBasePtr &_p) {
  ptr<Import> p = ast_must_cast<Import>(_p);

  ++p->_end_index; /// skip "import"
  auto rhs = peek(p->_end_index);
  if (rhs->get_node_type() != ASTNodeType::STRING_LITERAL) {
    error(p->_end_index, "Invalid import statement");
  }
  p->_end_index = parse_node(rhs);
  str filename = ast_must_cast<StringLiteral>(rhs)->get_value();
  p->set_filename(filename);
  return p->_end_index;
}
