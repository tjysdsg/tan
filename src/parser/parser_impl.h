#ifndef TAN_PARSER_IMPL_H
#define TAN_PARSER_IMPL_H
#include "base.h"
#include <memory>
#include <stack>
#include "token.h"
#include "src/ast/ast_node.h"

namespace tanlang {

class CompilerSession;
class ParsableASTNode;
using ParsableASTNodePtr = ptr<ParsableASTNode>;

/**
 * \brief Parser
 * \details Operator precedence parsing
 * */
class ParserImpl final {
public:
  ParserImpl() = delete;
  ParserImpl(vector<Token *> tokens, str filename, CompilerSession *cs);
  ParsableASTNodePtr parse();
  [[nodiscard]] str get_filename() const;

private:
  [[nodiscard]] Token *at(const size_t idx) const;
  bool eof(size_t index) const;
  ParsableASTNodePtr peek(size_t &index);
  ParsableASTNodePtr peek(size_t &index, TokenType type, const str &value);
  ParsableASTNodePtr peek_keyword(Token *token, size_t &index);
  ParsableASTNodePtr next_expression(size_t &index, int rbp = 0);
  size_t parse_node(const ParsableASTNodePtr &p);
  size_t parse_node(const ParsableASTNodePtr &left, const ParsableASTNodePtr &p);
  void error(size_t i, const str &error_message) const;
  size_t parse_intrinsic(const ParsableASTNodePtr &p);
  size_t parse_import(const ParsableASTNodePtr &p);
  size_t parse_if(const ParsableASTNodePtr &p);
  size_t parse_else(const ParsableASTNodePtr &p);
  size_t parse_loop(const ParsableASTNodePtr &p);
  size_t parse_func_decl(const ParsableASTNodePtr &p);
  size_t parse_func_call(const ParsableASTNodePtr &p);
  size_t parse_array_literal(const ParsableASTNodePtr &p);
  size_t parse_var_decl(const ParsableASTNodePtr &p);
  size_t parse_arg_decl(const ParsableASTNodePtr &p);
  size_t parse_struct_decl(const ParsableASTNodePtr &p);
  size_t parse_enum_decl(const ParsableASTNodePtr &p);
  size_t parse_member_access(const ParsableASTNodePtr &left, const ParsableASTNodePtr &p);

  size_t parse_ty(ParsableASTNodePtr &p);
  size_t parse_ty_array(const ASTTyPtr &p);
  size_t parse_ty_struct(const ASTTyPtr &p);

private:
  vector<Token *> _tokens{};
  str _filename = "";
  CompilerSession *_cs = nullptr;
  std::shared_ptr<ASTNode> _root = nullptr;
};

} // namespace tanlang

#endif /* TAN_PARSER_IMPL_H */
