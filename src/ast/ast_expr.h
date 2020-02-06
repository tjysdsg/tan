#ifndef TAN_SRC_AST_AST_EXPR_H_
#define TAN_SRC_AST_AST_EXPR_H_
#include "src/ast/astnode.h"

namespace tanlang {
struct Token;

class ASTParenthesis final : public ASTNode {
 public:
  ASTParenthesis() = delete;
  explicit ASTParenthesis(Token *token) : ASTNode(ASTType::PARENTHESIS,
                                                  op_precedence[ASTType::PARENTHESIS],
                                                  0, token) {};
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *parser_context) override;
};

class ASTArgDecl final : public ASTNode {
 public:
  ASTArgDecl() = delete;
  explicit ASTArgDecl(Token *token) : ASTNode(ASTType::ARG_DECL, 0, 0, token) {};
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *parser_context) override;
};

class ASTVarDecl final : public ASTNode {
 public:
  ASTVarDecl() = delete;
  explicit ASTVarDecl(Token *token) : ASTNode(ASTType::VAR_DECL, 0, 0, token) {};
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *parser_context) override;

 public:
  bool _has_initial_val = false;
};

}

#endif //TAN_SRC_AST_AST_EXPR_H_
