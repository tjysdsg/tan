#ifndef TAN_PARSER_H
#define TAN_PARSER_H
#include "base.h"
#include <memory>
#include <stack>
#include "src/ast/ast_node.h"

namespace tanlang {

class CompilerSession;
struct Token;
enum class TokenType;

/**
 * \brief Parser
 * \details Top-Down operator precedence parsing
 * */
class Parser final {
public:
  Parser() = delete;
  Parser(vector<Token *> tokens, const str &filename, CompilerSession *cs);
  std::shared_ptr<ASTNode> peek(size_t &index);
  std::shared_ptr<ASTNode> peek(size_t &index, TokenType type, const str &value);
  std::shared_ptr<ASTNode> next_expression(size_t &index, int rbp = 0);
  std::shared_ptr<ASTNode> parse();
  size_t parse_node(ASTNodePtr p);
  size_t parse_node(ASTNodePtr left, ASTNodePtr p);
  bool eof(size_t index) const;
  [[nodiscard]] Token *at(const size_t idx) const;
  [[nodiscard]] str get_filename() const;

protected:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  std::shared_ptr<ASTNode> _root = nullptr;
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
