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

void ASTInfixBinaryOp::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.emplace_back(left);
  auto n = parser->next_expression(_lbp);
  if (!n) {
    report_error();
  } else {
    _children.emplace_back(n);
  }
}

void ASTPrefix::nud(Parser *parser) {
  _children.emplace_back(parser->next_expression(_lbp));
}

/**
 * This is defined merely to overwrite ASTNode::nud() because the latter throws
 * */
void ASTNumberLiteral::nud(Parser *parser) {
  UNUSED(parser);
}

void ASTIdentifier::nud(Parser *parser) {
  auto *token = parser->get_curr_token();
  if (!_strict && token->type == TokenType::PUNCTUATION && token->value == "(") { // function call
    _op = ASTType::FUNC_CALL;
    ++parser->_curr_token;
    size_t token_size = parser->_tokens.size();
    while (parser->_curr_token < token_size) {
      _children.push_back(parser->next_expression());
      if (parser->get_curr_token()->type == TokenType::PUNCTUATION && parser->get_curr_token()->value == ",") {
        ++parser->_curr_token;
      } else { break; }
    }
    parser->advance(TokenType::PUNCTUATION, ")");
  }
}

}
