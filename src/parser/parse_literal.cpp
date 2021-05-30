#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/expr.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

// TODO: move type checking of elements to analysis phase
size_t ParserImpl::parse_array_literal(const ASTBasePtr &_p) {
  ptr<ArrayLiteral> p = ast_must_cast<ArrayLiteral>(_p);

  ++p->_end_index; /// skip '['

  if (at(p->_end_index)->value == "]") {
    // TODO: support empty array literal, but raise error if the type cannot be inferred
    error(p->_end_index, "Empty array literal");
  }

  auto element_type = ASTNodeType::INVALID;
  vector<ptr<Literal>> elements{};
  while (!eof(p->_end_index)) {
    if (at(p->_end_index)->value == ",") { /// skip ","
      ++p->_end_index;
      continue;
    } else if (at(p->_end_index)->value == "]") { /// skip "]"
      ++p->_end_index;
      break;
    }

    auto node = peek(p->_end_index);
    if (!is_ast_type_in(node->get_node_type(), TypeSystem::LiteralTypes)) {
      // TODO: support array of constexpr
      error(p->_end_index, "Expected a literal");
    }

    if (element_type == ASTNodeType::INVALID) { /// set the element type to first element if unknown
      element_type = node->get_node_type();
    } else { /// otherwise check whether element types are the same
      if (element_type != node->get_node_type()) {
        error(p->_end_index, "All elements in an array must have the same type");
      }
    }
    p->_end_index = parse_node(node);
    elements.push_back(ast_must_cast<Literal>(node));
  }

  p->set_elements(elements);
  return p->_end_index;
}
