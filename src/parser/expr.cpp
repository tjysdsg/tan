#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

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

void ASTIdentifier::nud(Parser *parser) {
  UNUSED(parser);
}

void ASTInfixBinaryOp::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.emplace_back(left);
  auto n = parser->next_expression(_lbp);
  if (!n) {
    report_error();
  } else {
    _children.emplace_back(n);
  }
}

/**
 * This defined only to overwrite ASTNode::nud() because the latter throws
 * */
void ASTNumberLiteral::nud(Parser *parser) {
  UNUSED(parser);
}

void ASTPrefix::nud(Parser *parser) {
  auto n = parser->next_expression(_lbp);
  if (!n) {
    throw std::runtime_error("Expect a token"); // FIXME: improve this error
  } else {
    _children.emplace_back(n);
  }
}

}
