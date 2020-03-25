#include "src/ast/ast_expr.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_array.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

void ASTParenthesis::nud(Parser *parser) {
  while (true) {
    Token *t = parser->get_curr_token();
    if (!t) {
      throw std::runtime_error("Unexpected EOF");
    } else if (t->type == TokenType::PUNCTUATION && t->value == ")") {
      ++parser->_curr_token;
      break;
    }
    auto n = parser->next_expression(PREC_LOWEST);
    if (n) {
      _children.push_back(n);
    } else {
      throw std::runtime_error("Unexpected " + t->to_string());
    }
  }
}

void ASTInfixBinaryOp::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.emplace_back(left);
  auto n = parser->next_expression(_lbp);
  if (!n) {
    report_code_error(_token, "Unexpected token");
  } else {
    _children.emplace_back(n);
  }
}

void ASTArrayLiteral::nud(Parser *parser) {
  if (parser->get_curr_token()->value == "]") { // empty array
    return;
  }
  ASTType element_type = ASTType::INVALID;
  while (!parser->eof()) {
    if (parser->get_curr_token()->value == ",") {
      ++parser->_curr_token;
      continue;
    } else if (parser->get_curr_token()->value == "]") {
      break;
    }
    auto node = parser->peek();
    if (!node) {
      report_code_error(_token, "Unexpected token");
    }
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
      if (node->_type == ASTType::ARRAY_LITERAL) { ++parser->_curr_token; }
      node->nud(parser); // TODO: set convention of parser->_curr_token before and after calling nud()/led()
      _children.push_back(node);
      ++parser->_curr_token;
    } else {
      report_code_error(_token, "Expect literals");
    }
  }
}

void ASTPrefix::nud(Parser *parser) {
  _children.emplace_back(parser->next_expression(_lbp));
}

void ASTAssignment::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.push_back(left);
  _children.push_back(parser->next_expression(0));
}

/**
 * This is defined merely to overwrite ASTNode::nud() because the latter throws
 * */
void ASTNumberLiteral::nud(Parser *parser) { UNUSED(parser); }

/**
 * This is defined merely to overwrite ASTNode::nud() because the latter throws
 * */
void ASTIdentifier::nud(Parser *parser) { UNUSED(parser); }

} // namespace tanlang
