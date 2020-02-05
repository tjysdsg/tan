#include "src/ast/ast_expr.h"
#include "src/parser/grammar_check.h"
#include "parser.h"

namespace tanlang {

void ASTArgDecl::nud(Parser *parser) {
  _children.push_back(parser->next_node()); // name
  parser->advance(TokenType::PUNCTUATION, ":");
  _children.push_back(parser->next_node()); // type
}

void ASTVarDecl::nud(Parser *parser) {
  _children.push_back(parser->next_node()); // name
  parser->advance(TokenType::PUNCTUATION, ":"); // TODO: type inference
  auto *curr = parser->get_curr_token();
  // FIXME: sophisticated types
  if (!check_typename_grammar(curr)) {
    report_code_error(curr->l, curr->c, "Expect a type, got " + curr->to_string() + " instead");
  }
  _children.push_back(parser->next_node()); // type

  curr = parser->get_curr_token();
  ++parser->_curr_token;
  if (curr->type == TokenType::BOP && curr->value == "=") { // initial value
    _children.push_back(parser->next_expression());
    _has_initial_val = true;
  }
}

}
