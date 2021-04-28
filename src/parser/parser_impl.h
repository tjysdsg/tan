#ifndef TAN_PARSER_IMPL_H
#define TAN_PARSER_IMPL_H
#include "base.h"
#include <memory>
#include <stack>
#include "token.h"
#include "src/ast/ast_node.h"

namespace tanlang {

class CompilerSession;

/**
 * \brief Parser
 * \details Operator precedence parsing
 * */
class ParserImpl final {
public:
  ParserImpl() = delete;
  ParserImpl(vector<Token *> tokens, str filename, CompilerSession *cs);
  ASTNodePtr parse();
  [[nodiscard]] str get_filename() const;

private:
  [[nodiscard]] Token *at(const size_t idx) const;
  bool eof(size_t index) const;
  ASTNodePtr peek(size_t &index);
  ASTNodePtr peek(size_t &index, TokenType type, const str &value);
  ASTNodePtr peek_keyword(Token *token, size_t &index);
  ASTNodePtr next_expression(size_t &index, int rbp = 0);
  size_t parse_node(const ASTNodePtr &p);
  size_t parse_node(const ASTNodePtr &left, const ASTNodePtr &p);
  void error(size_t i, const str &error_message) const;
  size_t parse_ty(const ASTNodePtr &p);
  size_t parse_ty_array(const ASTTyPtr &p);
  size_t parse_ty_struct(const ASTTyPtr &p);
  size_t parse_intrinsic(const ASTNodePtr &p);
  size_t parse_import(const ASTNodePtr &p);
  size_t parse_if(const ASTNodePtr &p);
  size_t parse_else(const ASTNodePtr &p);
  size_t parse_loop(const ASTNodePtr &p);
  size_t parse_func_decl(const ASTNodePtr &p);
  size_t parse_func_call(const ASTNodePtr &p);
  size_t parse_array_literal(const ASTNodePtr &p);
  size_t parse_var_decl(const ASTNodePtr &p);
  size_t parse_arg_decl(const ASTNodePtr &p);
  size_t parse_struct_decl(const ASTNodePtr &p);
  size_t parse_enum_decl(const ASTNodePtr &p);
  size_t parse_member_access(const ASTNodePtr &left, const ASTNodePtr &p);

private:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  std::shared_ptr<ASTNode> _root = nullptr;
};

} // namespace tanlang

#endif /* TAN_PARSER_IMPL_H */
