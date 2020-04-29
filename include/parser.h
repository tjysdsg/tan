#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "src/llvm_include.h"
#include "compiler_session.h"
#include "token.h"
#include "lexer.h"
#include <memory>
#include <stack>
#include <vector>

namespace tanlang {

class ASTNode;

struct Token;

class Parser {
public:
  Parser() = delete;
  Parser(std::vector<Token *> tokens, std::string filename);
  std::shared_ptr<ASTNode> peek(size_t &index);
  std::shared_ptr<ASTNode> peek(size_t &index, TokenType type, const std::string &value);
  std::shared_ptr<ASTNode> next_expression(size_t &index, int rbp = 0);
  std::shared_ptr<ASTNode> parse();
  bool eof(size_t index) const;
  [[nodiscard]] Token *at(const size_t idx) const;
  [[nodiscard]] std::string get_filename() const;
  [[nodiscard]] std::shared_ptr<ASTNode> get_ast() const;

public:
  std::shared_ptr<ASTNode> _root{};

protected:
  std::vector<Token *> _tokens;
  std::string _filename;

public:
  template<ASTType first_type> std::shared_ptr<ASTNode> parse(size_t &index, bool strict) {
    UNUSED(index);
    UNUSED(strict);
    /// NOTE: nothing here, everything is defined in specialized template functions in parser.hpp
    assert(false);
  }
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
