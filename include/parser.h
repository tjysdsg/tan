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
  ASTNodePtr peek(size_t &index);
  ASTNodePtr peek(size_t &index, TokenType type, const str &value);
  ASTNodePtr peek_keyword(Token *token, size_t &index);
  ASTNodePtr next_expression(size_t &index, int rbp = 0);
  ASTNodePtr parse();
  size_t parse_node(const ASTNodePtr &p);
  size_t parse_node(const ASTNodePtr &left, const ASTNodePtr &p);
  bool eof(size_t index) const;
  [[nodiscard]] Token *at(const size_t idx) const;
  [[nodiscard]] str get_filename() const;
  void error(const str &error_message);
  void error(size_t i, const str &error_message) const;

private:
  size_t parse_ty_array(const ASTTyPtr &p);
  size_t parse_ty_struct(const ASTTyPtr &p);

private:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  std::shared_ptr<ASTNode> _root = nullptr;
};

} // namespace tanlang

#endif /* TAN_PARSER_H */
