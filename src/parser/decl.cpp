#include <src/ast/ast_struct.h>

#include "src/ast/ast_expr.h"
#include "src/parser/token_check.h"
#include "parser.h"

namespace tanlang {

void ASTArgDecl::nud(Parser *parser) {
  _children.push_back(parser->parse<ASTType::ID>(true)); // name
  parser->advance(TokenType::PUNCTUATION, ":");
  _children.push_back(parser->parse<ASTType::TY>(true)); // type
}

void ASTVarDecl::nud(Parser *parser) {
  _children.push_back(parser->parse<ASTType::ID>(true)); // name
  parser->advance(TokenType::PUNCTUATION, ":"); // TODO: type inference
  _children.push_back(parser->parse<ASTType::TY>(true)); // type

  Token *curr = parser->get_curr_token();
  ++parser->_curr_token;
  // TODO: pass to ASTAssignment
  if (curr->type == TokenType::BOP && curr->value == "=") { // initial value
    _children.push_back(parser->next_expression());
    _has_initial_val = true;
  }
}

void ASTStruct::nud(Parser *parser) {
  _children.push_back(parser->next_expression()); // name
  auto comp_statements = parser->advance();
  comp_statements->nud(parser);
  ++parser->_curr_token;
  _children.insert(_children.begin() + 1, comp_statements->_children.begin(), comp_statements->_children.end());
}

}
