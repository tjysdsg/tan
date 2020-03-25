#ifndef TAN_SRC_AST_AST_EXPR_H_
#define TAN_SRC_AST_AST_EXPR_H_
#include "src/ast/astnode.h"
#include "astnode.h"
#include "src/ast/ast_ty.h"

namespace tanlang {
struct Token;

class ASTParenthesis final : public ASTNode {
public:
  ASTParenthesis() = delete;

  explicit ASTParenthesis(Token *token) : ASTNode(ASTType::PARENTHESIS, op_precedence[ASTType::PARENTHESIS], 0, token
  ) {};
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

class ASTVarDecl final
    : public ASTNode, public std::enable_shared_from_this<ASTVarDecl>, public Named, public Typed, public Valued {
public:
  ASTVarDecl() = default;

  friend class ASTFunction; // FIXME: is this necessary?

  explicit ASTVarDecl(Token *token) : ASTNode(ASTType::VAR_DECL, 0, 0, token) {};
  void nud(Parser *parser) override;

  /**
   * \attention UNUSED
   * \internal
   * */
  Value *codegen(CompilerSession *compiler_session) override;

  std::string get_name() const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *compiler_session) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;

public:
  bool _has_initial_val = false;
private:
  Value *_llvm_value = nullptr;
  std::shared_ptr<ASTTy> _ty = nullptr;
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
