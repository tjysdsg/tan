#ifndef TAN_PARSER_IMPL_H
#define TAN_PARSER_IMPL_H
#include "base.h"
#include <memory>
#include <stack>
#include "token.h"
#include "src/ast/fwd.h"

namespace tanlang {

/**
 * \brief Parser
 * \details Operator precedence parsing
 * */
class ParserImpl final {
public:
  ParserImpl() = delete;
  ParserImpl(vector<Token *> tokens, str filename, CompilerSession *cs);
  ASTBase *parse();
  [[nodiscard]] str get_filename() const;

private:
  [[nodiscard]] Token *at(const size_t idx) const;
  bool eof(size_t index) const;
  ASTBase *peek(size_t &index);
  ASTBase *peek(size_t &index, TokenType type, const str &value);
  ASTBase *peek_keyword(Token *token, size_t &index);
  ASTBase *next_expression(size_t &index, int rbp);
  size_t parse_node(ASTBase *p);
  size_t parse_node(ASTBase *left, ASTBase *p);

  Expr *expect_expression(ASTBase *p);
  Stmt *expect_stmt(ASTBase *p);
  Decl *expect_decl(ASTBase *p);

  size_t parse_program(ASTBase *p);
  size_t parse_stmt(ASTBase *p);
  size_t parse_intrinsic(ASTBase *p);
  size_t parse_import(ASTBase *p);
  size_t parse_if(ASTBase *p);
  size_t parse_loop(ASTBase *p);
  size_t parse_func_decl(ASTBase *p);
  size_t parse_func_call(ASTBase *p);
  size_t parse_array_literal(ASTBase *p);
  size_t parse_var_decl(ASTBase *p);
  size_t parse_arg_decl(ASTBase *p);
  size_t parse_struct_decl(ASTBase *p);
  // size_t parse_enum_decl(const ASTBase * &p);
  size_t parse_uop(ASTBase *p);
  size_t parse_return(ASTBase *p);
  size_t parse_parenthesis(ASTBase *p);

  size_t parse_member_access(Expr *left, MemberAccess *p);
  size_t parse_cast(ASTBase *left, ASTBase *p);
  size_t parse_assignment(ASTBase *left, ASTBase *p);
  size_t parse_bop(ASTBase *left, ASTBase *p);

  size_t parse_ty(ASTType *p);
  size_t parse_ty_array(ASTType *p);

  [[noreturn]] void error(size_t i, const str &error_message) const;

private:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  ASTBase *_root = nullptr;
};

} // namespace tanlang

#endif /* TAN_PARSER_IMPL_H */
