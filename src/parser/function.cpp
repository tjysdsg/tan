#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

void ASTFunction::nud(Parser *parser) {
  _children.push_back(std::make_shared<ASTNode>()); // function return type, set later
  _children.push_back(parser->next_node()); // function name
  parser->advance(TokenType::PUNCTUATION, "(");
  // if the argument list isn't empty
  if (parser->get_curr_token()->type != TokenType::PUNCTUATION || parser->get_curr_token()->value != ")") {
    size_t token_size = parser->_tokens.size();
    while (parser->_curr_token < token_size) {
      std::shared_ptr<ASTNode> arg = std::make_shared<ASTArgDecl>(parser->get_curr_token());
      arg->nud(parser);
      _children.push_back(arg);
      if (parser->get_curr_token()->type == TokenType::PUNCTUATION && parser->get_curr_token()->value == ",") {
        ++parser->_curr_token;
      } else { break; }
    }
  }
  parser->advance(TokenType::PUNCTUATION, ")");
  parser->advance(TokenType::PUNCTUATION, ":");
  // figure out function return type
  _children[0] = parser->next_node();

  // function code
  std::shared_ptr<ASTNode> code = std::make_shared<ASTStatement>(parser->get_curr_token());
  code->nud(parser);
  _children.push_back(code);
}

}
