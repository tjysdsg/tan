#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

void ASTFunction::nud(Parser *parser) {
  _children.push_back(std::make_shared<ASTNode>()); // function return type, set later
  _children.push_back(parser->parse<ASTType::ID>(true)); // function name
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
  _children[0] = parser->parse<ASTType::TYPENAME>(true); // return type

  // get function body if exists, otherwise it's a external function
  auto *token = parser->get_curr_token();
  if (token->type == TokenType::PUNCTUATION && token->value == "{") {
    std::shared_ptr<ASTNode> code = std::make_shared<ASTStatement>(token);
    code->nud(parser);
    _children.push_back(code);
    _is_external = false;
  } else {
    ++parser->_curr_token;
    _is_external = true;
  }
}

void ASTFunctionCall::nud(Parser *parser) {
  if (_parsed) {
    parser->_curr_token = _parsed_index;
    return;
  }
  auto *token = parser->get_curr_token();
  if (token->type != TokenType::PUNCTUATION || token->value != "(") { // function call
    report_code_error(token->l, token->c, "Invalid function call");
  }
  ++parser->_curr_token;
  size_t token_size = parser->_tokens.size();
  while (parser->_curr_token < token_size) {
    _children.push_back(parser->next_expression());
    if (parser->get_curr_token()->type == TokenType::PUNCTUATION && parser->get_curr_token()->value == ",") {
      ++parser->_curr_token;
    } else { break; }
  }
  parser->advance(TokenType::PUNCTUATION, ")");
  _parsed = true;
  _parsed_index = parser->_curr_token;
}

} // namespace tanlang
