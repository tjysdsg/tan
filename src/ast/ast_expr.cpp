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
Value *ASTParenthesis::codegen(ParserContext *parser_context) {
  auto *result = _children[0]->codegen(parser_context);
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) {
    _children[i]->codegen(parser_context);
  }
  return result;
}

void ASTArgDef::nud(Parser *parser) {
  _children.push_back(parser->next_node()); // name
  parser->advance(TokenType::PUNCTUATION, ":");
  _children.push_back(parser->next_node()); // type
}

Value *ASTArgDef::codegen(ParserContext *parser_context) {
  return ASTNode::codegen(parser_context);
}

}
