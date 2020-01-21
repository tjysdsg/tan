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
  } else if (token->type == TokenType::RELOP) {
    if (token->value == ">") {
      node = new ASTCompare(ASTType::GT);
    } else if (token->value == ">=") {
      node = new ASTCompare(ASTType::GE);
    } else if (token->value == "<") {
      node = new ASTCompare(ASTType::LT);
    } else if (token->value == "<=") {
      node = new ASTCompare(ASTType::LE);
    }
  } else if (token->type == TokenType::INT) {
    node = new ASTNumberLiteral(token->value, false);
  } else if (token->type == TokenType::FLOAT) {
    node = new ASTNumberLiteral(token->value, true);
  } else if (token->type == TokenType::STRING) {
    node = new ASTStringLiteral(token->value);
  } else if (token->type == TokenType::KEYWORD && token->value == "return") {
    node = new ASTReturn;
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") {
    node = new ASTStatement(true);
  } else if (token->type == TokenType::PUNCTUATION && (token->value == ";" || token->value == "}")) {
    return nullptr; // FIXME: nullptr represent a terminal symbol, like statements ending with a semicolon
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

ASTNode *Parser::next_statement() {
  auto *statement = new ASTStatement();
  ASTNode *node = peek();
  while (node) {
    statement->_children.push_back(next_expression(0));
    node = peek();
  }
  return statement;
}

ASTNode *Parser::parse() {
  _root = new ASTProgram;
  return _root->nud(this);
}

Parser::~Parser() {
  delete _root;
  _root = nullptr;
}

}
