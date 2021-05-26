#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/parsable_ast_node.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

size_t ParserImpl::parse_array_literal(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip '['
  if (at(p->_end_index)->value == "]") { error(p->_end_index, "Empty array"); }
  auto element_type = ASTType::INVALID;
  while (!eof(p->_end_index)) {
    if (at(p->_end_index)->value == ",") {
      ++p->_end_index;
      continue;
    } else if (at(p->_end_index)->value == "]") {
      ++p->_end_index;
      break;
    }
    auto node = peek(p->_end_index);
    if (!node) { error(p->_end_index, "Unexpected token"); }
    /// check whether element types are the same
    if (element_type == ASTType::INVALID) { element_type = node->get_node_type(); }
    else {
      if (element_type != node->get_node_type()) {
        error(p->_end_index, "All elements in an array must have the same type");
      }
    }
    if (is_ast_type_in(node->get_node_type(), TypeSystem::LiteralTypes)) {
      if (node->get_node_type() == ASTType::ARRAY_LITERAL) { ++p->_end_index; }
      p->_end_index = parse_node(node);
      p->append_child(node);
    } else { error(p->_end_index, "Expect literals"); }
  }

  return p->_end_index;
}
