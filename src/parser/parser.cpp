#include "parser.h"
#include <vector>
#include "ast.h"

using std::string;

namespace tanlang {

ASTNode *Parser::advance() {
  auto *r = peek();
  ++_curr_token;
  return r;
}

ASTNode *Parser::peek() {
  if (_curr_token >= _tokens.size()) return nullptr;
  Token *token = _tokens[_curr_token];
  ASTNode *node = nullptr;
  if (token->value == "+" && token->type == TokenType::BOP) {
    node = new ASTSum;
  } else if (token->value == "-" && token->type == TokenType::BOP) {
    node = new ASTSubtract;
  } else if (token->value == "*" && token->type == TokenType::BOP) {
    node = new ASTMultiply;
  } else if (token->value == "/" && token->type == TokenType::BOP) {
    node = new ASTDivide;
  } else if (token->value == "!" && token->type == TokenType::UOP) {
    node = new ASTLogicalNot;
  } else if (token->value == "~" && token->type == TokenType::UOP) {
    node = new ASTBinaryNot;
  } else if (token->type == TokenType::INT) {
    node = new ASTNumberLiteral(token->value, false);
  } else if (token->type == TokenType::FLOAT) {
    node = new ASTNumberLiteral(token->value, true);
  } else if (token->type == TokenType::STRING) {
    node = new ASTStringLiteral(token->value);
  } else {
    throw std::runtime_error("unknown token " + token->to_string());
  }
  return node;
}

ASTNode *Parser::next_expression(int rbp) {
  ASTNode *node = advance();
  if (!node) {
    return nullptr;
  }
  auto *n = node;
  ASTNode *left = n->nud(this);
  node = peek();
  if (!node) {
    return left;
  }
  while (rbp < node->_lbp) {
    node = peek();
    n = node;
    ++_curr_token;
    left = n->led(left, this);
    node = peek();
    if (!node) { break; };
  }
  return left;
}

ASTNode *Parser::parse() {
  size_t n_tokens = _tokens.size();
  _root = new ASTProgram;
  while (_curr_token < n_tokens) {
    auto *n = next_expression(0);
    if (!n) { break; }
    _root->add(n);
  }
  return _root;
}

Parser::~Parser() {
  delete _root;
  _root = nullptr;
}

}
