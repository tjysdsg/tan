#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "base.h"
#include <memory>
#include <stack>

namespace tanlang {

class ASTNode;
class CompilerSession;
struct Token;
enum class TokenType;
enum class ASTType;

class Parser {
public:
  Parser() = delete;
  Parser(std::vector<Token *> tokens, const str &filename, CompilerSession *cs);
  std::shared_ptr<ASTNode> peek(size_t &index);
  std::shared_ptr<ASTNode> peek(size_t &index, TokenType type, const str &value);
  std::shared_ptr<ASTNode> next_expression(size_t &index, int rbp = 0);
  std::shared_ptr<ASTNode> parse();
  bool eof(size_t index) const;
  [[nodiscard]] Token *at(const size_t idx) const;
  [[nodiscard]] str get_filename() const;
  [[nodiscard]] std::shared_ptr<ASTNode> get_ast() const;

public:
  std::shared_ptr<ASTNode> _root = nullptr;

protected:
  std::vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;

public:
  /// NOTE: nothing here, everything is defined in specialized template functions in parser.hpp
  template<ASTType type> std::shared_ptr<ASTNode> parse(size_t &index, bool strict);
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
