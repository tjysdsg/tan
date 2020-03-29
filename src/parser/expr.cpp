#include "src/ast/ast_expr.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_array.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

size_t ASTParenthesis::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip (
  while (true) {
    auto *t = parser->at(_end_index);
    if (!t) {
      throw std::runtime_error("Unexpected EOF");
    } else if (t->type == TokenType::PUNCTUATION && t->value == ")") { /// end at )
      ++_end_index;
      break;
    }
    auto n = parser->next_expression(_end_index, PREC_LOWEST);
    if (n) {
      _children.push_back(n);
    } else {
      throw std::runtime_error("Unexpected " + t->to_string());
    }
  }
  return _end_index;
}

size_t ASTInfixBinaryOp::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip operator
  _children.emplace_back(left); /// lhs
  auto n = parser->next_expression(_end_index, _lbp);
  if (!n) {
    report_code_error(_token, "Unexpected token");
  } else {
    _children.emplace_back(n);
  }
  return _end_index;
}

size_t ASTArrayLiteral::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip '['
  if (parser->at(_end_index)->value == "]") { /// empty array
    ++_end_index;
    return _end_index;
  }
  ASTType element_type = ASTType::INVALID;
  while (!parser->eof(_end_index)) {
    if (parser->at(_end_index)->value == ",") {
      ++_end_index;
      continue;
    } else if (parser->at(_end_index)->value == "]") {
      ++_end_index;
      break;
    }
    auto node = parser->peek(_end_index);
    if (!node) { report_code_error(_token, "Unexpected token"); }
    /// check whether element types are the same
    if (element_type == ASTType::INVALID) {
      element_type = node->_type;
    } else {
      if (element_type != node->_type) {
        report_code_error(_token, "All elements in an array must have the same type");
      }
    }
    if (node->_type == ASTType::NUM_LITERAL || node->_type == ASTType::STRING_LITERAL
        || node->_type == ASTType::ARRAY_LITERAL) { // FIXME: More literals?
      if (node->_type == ASTType::ARRAY_LITERAL) { ++_end_index; }
      _end_index = node->parse(parser);
      _children.push_back(node);
    } else {
      report_code_error(_token, "Expect literals");
    }
  }
  return _end_index;
}

size_t ASTStringLiteral::nud(Parser *parser) {
  UNUSED(parser);
  _end_index = _start_index + 1; /// skip self
  return _end_index;
}

size_t ASTPrefix::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip self
  _children.emplace_back(parser->next_expression(_end_index, _lbp));
  return _end_index;
}

size_t ASTAssignment::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip "="
  _children.push_back(left);
  _children.push_back(parser->next_expression(_end_index, 0));
  return _end_index;
}

size_t ASTArithmetic::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "-"
  auto lhs = std::make_shared<ASTNumberLiteral>("0", false, nullptr, _start_index);
  _children.push_back(lhs);
  _children.push_back(parser->next_expression(_end_index, 0));
  return _end_index;
}

size_t ASTNumberLiteral::nud(Parser *parser) {
  _end_index = _start_index + 1;
  UNUSED(parser);
  return _end_index;
}

size_t ASTIdentifier::nud(Parser *parser) {
  _end_index = _start_index + 1;
  UNUSED(parser);
  return _end_index;
}

} // namespace tanlang
