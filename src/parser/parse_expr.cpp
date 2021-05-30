#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/expr.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

// TODO: move type checking of array elements to analysis phase
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

size_t ParserImpl::parse_bop(const ASTBasePtr &_lhs, const ASTBasePtr &_p) {
  ptr<Expr> lhs = ast_must_cast<Expr>(_lhs);
  ptr<BinaryOperator> p = ast_must_cast<BinaryOperator>(_p);

  if (_p->get_token_str() == "." || _p->get_token_str() == "[") { /// delegate to parse_member_access
    return parse_member_access(lhs, p);
  }

  ++p->_end_index; /// skip the operator

  p->set_lhs(lhs); /// lhs

  /// rhs
  auto rhs = next_expression(p->_end_index, p->get_lbp());
  if (!rhs) {
    error(p->_end_index, "Invalid operand");
  }
  p->set_rhs(rhs);

  return p->_end_index;
}

size_t ParserImpl::parse_uop(const ASTBasePtr &_p) {
  ptr<UnaryOperator> p = ast_must_cast<UnaryOperator>(_p);

  /// rhs
  ++p->_end_index;
  auto rhs = ast_cast<Expr>(next_expression(p->_end_index, p->get_lbp()));
  if (!rhs) {
    error(p->_end_index, "Invalid operand");
  }
  p->set_rhs(rhs);

  return p->_end_index;
}

size_t ParserImpl::parse_parenthesis(const ASTBasePtr &_p) {
  ptr<Parenthesis> p = ast_must_cast<Parenthesis>(_p);

  ++p->_end_index; /// skip "("
  while (true) {
    auto *t = at(p->_end_index);
    if (!t) {
      error(p->_end_index - 1, "Unexpected EOF");
    } else if (t->type == TokenType::PUNCTUATION && t->value == ")") { /// end at )
      ++p->_end_index;
      break;
    }
    // FIXME: multiple expressions in the parenthesis?

    /// NOTE: parenthesis without child expression inside are illegal (except function call)
    auto _sub = next_expression(p->_end_index, PREC_LOWEST);
    ptr<Expr> sub = nullptr;
    if (sub = ast_cast<Expr>(_sub)) {
      p->set_sub(sub);
    } else {
      error(p->_end_index, "Expect an expression");
    }
  }
  return 0;
}
