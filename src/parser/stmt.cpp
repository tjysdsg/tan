#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

// =================== if ===================//
void ASTIf::nud(tanlang::Parser *parser) {
  // condition
  auto condition = parser->advance(TokenType::PUNCTUATION, "(");
  condition->nud(parser);
  _children.push_back(condition);
  // if clause
  auto if_clause = parser->advance(TokenType::PUNCTUATION, "{");
  if_clause->nud(parser);
  _children.push_back(if_clause);
  ++parser->_curr_token;

  // else clause
  Token *token = parser->get_curr_token();
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    auto else_clause = parser->advance();
    else_clause->nud(parser);
    _children.push_back(else_clause); // else clause
    _has_else = true;
  }
}

// =================== else ===================//
void ASTElse::nud(tanlang::Parser *parser) {
  auto else_clause = parser->advance(TokenType::PUNCTUATION, "{");
  else_clause->nud(parser);
  _children.push_back(else_clause);
}

/**
 * \brief: parse a statement if _is_compound is false, otherwise parse a list of (compound) statements and add them
 *          to _children.
 * */
void ASTStatement::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  if (_is_compound) {
    while (parser->_curr_token < n_tokens) {
      auto n = parser->next_statement();
      if (!n || n->_children.empty()) { break; }
      _children.push_back(n);
      ++parser->_curr_token;
    }
  } else {
    auto n = std::reinterpret_pointer_cast<ASTStatement>(parser->next_statement());
    if (n && !n->_children.empty()) {
      *this = *n;
      ++parser->_curr_token;
    }
  }
  // TODO: increment parser->_curr_token, and adjust relevant calls to this function
}

}
