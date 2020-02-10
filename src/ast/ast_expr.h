#ifndef TAN_SRC_AST_AST_EXPR_H_
#define TAN_SRC_AST_AST_EXPR_H_
#include "src/ast/astnode.h"
#include "astnode.h"

namespace tanlang {
struct Token;

class ASTParenthesis final : public ASTNode {
 public:
  ASTParenthesis() = delete;
  explicit ASTParenthesis(Token *token) : ASTNode(ASTType::PARENTHESIS,
                                                  op_precedence[ASTType::PARENTHESIS],
                                                  0, token) {};
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTArgDecl final : public ASTNode {
 public:
  ASTArgDecl() = delete;
  explicit ASTArgDecl(Token *token) : ASTNode(ASTType::ARG_DECL, 0, 0, token) {};
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTVarDecl final : public ASTNode {
 public:
  ASTVarDecl() = delete;
  explicit ASTVarDecl(Token *token) : ASTNode(ASTType::VAR_DECL, 0, 0, token) {};
  void nud(Parser *parser) override;

  /**
   * \attention UNUSED
   * \internal
   * */
  Value *codegen(CompilerSession *compiler_session) override;

 public:
  bool _has_initial_val = false;
};

class ASTNumberLiteral final : public ASTNode {
 public:
  ASTNumberLiteral(const std::string &str, bool is_float, Token *token);
  void nud(Parser *parser) override;
  [[nodiscard]] bool is_float() const { return _is_float; }
  Value *codegen(CompilerSession *compiler_session) override;

 private:
  bool _is_float = false;
  union {
    int _ivalue;
    float _fvalue;
  };
};

class ASTStringLiteral final : public ASTNode {
 public:
  explicit ASTStringLiteral(std::string str, Token *token);

 private:
  std::string _svalue;
};

class ASTAssignment final : public ASTInfixBinaryOp {
 public:
  explicit ASTAssignment(Token *token);
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_EXPR_H_
