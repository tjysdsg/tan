#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_base.h"

using namespace tanlang;

size_t ParserImpl::parse_struct_decl(const ASTBasePtr &p) {
  ++p->_end_index; /// skip "struct"

  /// struct name
  auto id = peek(p->_end_index);
  if (id->get_node_type() != ASTNodeType::ID) {
    error(p->_end_index, "Expect struct name");
  }
  p->set_data(id->get_data<str>());

  /// struct body
  if (at(p->_end_index)->value == "{") {
    auto comp_stmt = next_expression(p->_end_index);
    if (!comp_stmt || comp_stmt->get_node_type() != ASTNodeType::STATEMENT) {
      error(comp_stmt->_end_index, "Invalid struct body");
    }

    // copy children
    size_t n_children = comp_stmt->get_children_size();
    for (size_t i = 0; i < n_children; ++i) {
      p->set_child_at(i, comp_stmt->get_child_at(i));
    }
  }

  return p->_end_index;
}
