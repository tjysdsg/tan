#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/factory.h"

using namespace tanlang;

size_t ParserImpl::parse_enum_decl(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip "enum"
  auto name = peek(p->_end_index);
  if (name->get_node_type() != ASTType::ID) {
    error(p->_end_index, "Expect an enum name");
  }

  /// enum body
  if (at(p->_end_index)->value != "{") {
    error(p->_end_index, "Invalid enum declaration");
  }
  ++p->_end_index;
  while (!eof(p->_end_index) && at(p->_end_index)->value != "}") {
    auto e = ast_create_statement(_cs);
    p->append_child(e);
    if (at(p->_end_index)->value == ",") { ++p->_end_index; }
  }
  ++p->_end_index; /// skip '}'

  return p->_end_index;
}
