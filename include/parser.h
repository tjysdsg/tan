#ifndef TAN_PARSER_H
#define TAN_PARSER_H

#include "lexer.h"
#include <vector>

namespace tanlang {

class ASTNode;

class Parser final {
 public:
  Parser() = delete;

  explicit Parser(std::vector<Token *> tokens) : _tokens(std::move(tokens)), _curr_token(0) {}

  ~Parser();

  ASTNode *advance();
  ASTNode *peek();
  ASTNode *next_expression(int rbp = 0);
  ASTNode *parse();

  std::vector<Token *> _tokens;
  ASTNode *_root = nullptr;
  size_t _curr_token;
};
}

#endif /* TAN_PARSER_H */
