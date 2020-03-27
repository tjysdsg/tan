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

  ASTParenthesis(Token *token, size_t token_index) : ASTNode(ASTType::PARENTHESIS,
                                                             op_precedence[ASTType::PARENTHESIS],
                                                             0,
                                                             token,
                                                             token_index
  ) {};
  size_t nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTArgDecl final : public ASTNode {
public:
  ASTArgDecl() = delete;

  ASTArgDecl(Token *token, size_t token_index) : ASTNode(ASTType::ARG_DECL, 0, 0, token, token_index) {};
  size_t nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTVarDecl final
    : public ASTNode, public std::enable_shared_from_this<ASTVarDecl>, public Named, public Typed, public Valued {
public:
  friend class ASTFunction; // FIXME: is this necessary?
  ASTVarDecl() = delete;

  // TODO: delete this and fix relevant callers
  explicit ASTVarDecl(size_t token_index) : ASTNode(ASTType::VAR_DECL, 0, 0, nullptr, token_index) {}

  ASTVarDecl(Token *token, size_t token_index) : ASTNode(ASTType::VAR_DECL, 0, 0, token, token_index) {};
  size_t nud(Parser *parser) override;

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

class ASTNumberLiteral final : public ASTLiteral {
public:
  friend class ASTTy;

public:
  ASTNumberLiteral(const std::string &str, bool is_float, Token *token, size_t token_index);
  size_t nud(Parser *parser) override;

  [[nodiscard]] bool is_float() const { return _is_float; }

  Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *compiler_session) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  Ty get_ty() const override;
  std::string to_string(bool print_prefix = true) const override;
private:
  bool _is_float = false;
  union {
    int _ivalue;
    float _fvalue;
  };
  llvm::Value *_llvm_value = nullptr;
};

class ASTStringLiteral final : public ASTLiteral {
public:
  ASTStringLiteral() = delete;
  ASTStringLiteral(Token *token, size_t token_index);
  size_t nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  Ty get_ty() const override;

private:
  std::string _svalue;
  llvm::Value *_llvm_value = nullptr;
  llvm::Type *_llvm_type = nullptr;
};

class ASTAssignment final : public ASTInfixBinaryOp {
public:
  ASTAssignment(Token *token, size_t token_index);
  size_t led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_EXPR_H_
