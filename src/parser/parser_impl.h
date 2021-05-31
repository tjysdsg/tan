#ifndef TAN_PARSER_IMPL_H
#define TAN_PARSER_IMPL_H
#include "base.h"
#include <memory>
#include <stack>
#include "token.h"

namespace tanlang {

class CompilerSession;
AST_FWD_DECL(ASTBase);
AST_FWD_DECL(ASTType);
AST_FWD_DECL(Expr);
AST_FWD_DECL(Decl);
AST_FWD_DECL(Stmt);
AST_FWD_DECL(MemberAccess);

/**
 * \brief Parser
 * \details Operator precedence parsing
 * */
class ParserImpl final {
public:
  ParserImpl() = delete;
  ParserImpl(vector<Token *> tokens, str filename, CompilerSession *cs);
  ASTBasePtr parse();
  [[nodiscard]] str get_filename() const;

private:
  [[nodiscard]] Token *at(const size_t idx) const;
  bool eof(size_t index) const;
  ASTBasePtr peek(size_t &index);
  ASTBasePtr peek(size_t &index, TokenType type, const str &value);
  ASTBasePtr peek_keyword(Token *token, size_t &index);
  ASTBasePtr next_expression(size_t &index, int rbp);
  size_t parse_node(const ASTBasePtr &p);
  size_t parse_node(const ASTBasePtr &left, const ASTBasePtr &p);

  ExprPtr expect_expression(const ASTBasePtr &p);
  StmtPtr expect_stmt(const ASTBasePtr &p);
  DeclPtr expect_decl(const ASTBasePtr &p);

  size_t parse_program(const ASTBasePtr &p);
  size_t parse_stmt(const ASTBasePtr &p);
  size_t parse_intrinsic(const ASTBasePtr &p);
  size_t parse_import(const ASTBasePtr &p);
  size_t parse_if(const ASTBasePtr &p);
  size_t parse_loop(const ASTBasePtr &p);
  size_t parse_func_decl(const ASTBasePtr &p);
  size_t parse_func_call(const ASTBasePtr &p);
  size_t parse_array_literal(const ASTBasePtr &p);
  size_t parse_var_decl(const ASTBasePtr &p);
  size_t parse_arg_decl(const ASTBasePtr &p);
  size_t parse_struct_decl(const ASTBasePtr &p);
  // size_t parse_enum_decl(const ASTBasePtr &p);
  size_t parse_uop(const ASTBasePtr &p);
  size_t parse_return(const ASTBasePtr &p);
  size_t parse_parenthesis(const ASTBasePtr &p);

  size_t parse_member_access(const ptr<Expr> &left, const ptr<MemberAccess> &p);
  size_t parse_cast(const ASTBasePtr &left, const ASTBasePtr &p);
  size_t parse_assignment(const ASTBasePtr &left, const ASTBasePtr &p);
  size_t parse_bop(const ASTBasePtr &left, const ASTBasePtr &p);

  size_t parse_ty(const ASTTypePtr &p);
  size_t parse_ty_array(const ASTTypePtr &p);

  void error(size_t i, const str &error_message) const;

private:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  ASTBasePtr _root = nullptr;
};

} // namespace tanlang

#endif /* TAN_PARSER_IMPL_H */
