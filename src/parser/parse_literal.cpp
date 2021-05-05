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
    if (element_type == ASTType::INVALID) { element_type = node->_type; }
    else {
      if (element_type != node->_type) {
        error(p->_end_index, "All elements in an array must have the same type");
      }
    }
    if (is_ast_type_in(node->_type, TypeSystem::LiteralTypes)) {
      if (node->_type == ASTType::ARRAY_LITERAL) { ++p->_end_index; }
      p->_end_index = parse_node(node);
      p->append_child(node);
    } else { error(p->_end_index, "Expect literals"); }
  }

  return p->_end_index;
}
