#include "parser.h"
#include <vector>
#include <memory>
#include "ast.h"

using std::string;

namespace tanlang {

std::shared_ptr<ASTNode> Parser::advance() {
  auto r = peek();
  ++_curr_token;
  return r;
}

std::shared_ptr<ASTNode> Parser::peek() {
  if (_curr_token >= _tokens.size()) return nullptr;
  Token *token = _tokens[_curr_token];
  std::shared_ptr<ASTNode> node;
  if (token->value == "+" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTSum>();
  } else if (token->value == "-" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTSubtract>();
  } else if (token->value == "*" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTMultiply>();
  } else if (token->value == "/" && token->type == TokenType::BOP) {
    node = std::make_shared<ASTDivide>();
  } else if (token->value == "!" && token->type == TokenType::UOP) {
    node = std::make_shared<ASTLogicalNot>();
  } else if (token->value == "~" && token->type == TokenType::UOP) {
    node = std::make_shared<ASTBinaryNot>();
  } else if (token->type == TokenType::RELOP) {
    if (token->value == ">") {
      node = std::make_shared<ASTCompare>(ASTType::GT);
    } else if (token->value == ">=") {
      node = std::make_shared<ASTCompare>(ASTType::GE);
    } else if (token->value == "<") {
      node = std::make_shared<ASTCompare>(ASTType::LT);
    } else if (token->value == "<=") {
      node = std::make_shared<ASTCompare>(ASTType::LE);
    }
  } else if (token->type == TokenType::INT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, false);
  } else if (token->type == TokenType::FLOAT) {
    node = std::make_shared<ASTNumberLiteral>(token->value, true);
  } else if (token->type == TokenType::STRING) {
    node = std::make_shared<ASTStringLiteral>(token->value);
  } else if (token->type == TokenType::KEYWORD && token->value == "return") {
    node = std::make_shared<ASTReturn>();
  } else if (token->type == TokenType::PUNCTUATION && token->value == "{") {
    node = std::make_shared<ASTStatement>(true);
  } else if (token->type == TokenType::PUNCTUATION && (token->value == ";" || token->value == "}")) {
    return nullptr; // FIXME: nullptr represent a terminal symbol, like statements ending with a semicolon
  } else {
    throw std::runtime_error("unknown token " + token->to_string() + "; LINE " + std::to_string(token->l) + ":COL "
                                 + std::to_string(token->c));
  }
  return node;
}

std::shared_ptr<ASTNode> Parser::next_expression(int rbp) {
  std::shared_ptr<ASTNode> node = advance();
  if (!node) {
    return nullptr;
  }
  auto n = node;
  n->nud(this);
  auto left = n;
  node = peek();
  if (!node) {
    return left;
  }
  while (rbp < node->_lbp) {
    node = peek();
    n = node;
    ++_curr_token;
    n->led(left, this);
    left = n;
    node = peek();
    if (!node) { break; };
  }
  return left;
}

/**
 * \brief: parse a single non-compound statement
 * \note: since a statement parsing might fail, _curr_token is one before the next new token to be parsed. Therefore,
 *          remember to increment _curr_token after successfully finishing a next_statement() call!
 * */
std::shared_ptr<ASTNode> Parser::next_statement() {
  auto statement = std::make_shared<ASTStatement>();
  auto node = peek();
  while (node) {
    statement->_children.push_back(next_expression(0));
    node = peek();
  }
  return statement;
}

std::shared_ptr<ASTNode> Parser::parse() {
  _root = std::make_shared<ASTProgram>();
  _root->nud(this);
  return _root;
}

}
