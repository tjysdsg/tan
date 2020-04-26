#ifndef TAN_SRC_AST_AST_EXPR_H_
#define TAN_SRC_AST_AST_EXPR_H_
#include "src/ast/astnode.h"
#include "src/ast/ast_ty.h"

namespace tanlang {

struct Token;

class ASTParenthesis final : public ASTNode {
public:
  ASTParenthesis() = delete;
  ASTParenthesis(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  bool is_typed() const override;
  bool is_lvalue() const override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Metadata *to_llvm_meta(CompilerSession *) const override;

protected:
  size_t nud(Parser *parser) override;
};

class ASTVarDecl : public ASTNode, public std::enable_shared_from_this<ASTVarDecl> {
public:
  friend class ASTAssignment;

  friend class ASTFunction;

  ASTVarDecl() = delete;

  ASTVarDecl(Token *token, size_t token_index) : ASTNode(ASTType::VAR_DECL, 0, 0, token, token_index) {};

  Value *codegen(CompilerSession *compiler_session) override;

  bool is_typed() const override { return true; }

  bool is_named() const override { return true; }

  std::string get_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *compiler_session) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;

  bool is_lvalue() const override { return true; }

protected:
  size_t nud(Parser *parser) override;
  size_t _nud(Parser *parser);

public:
  bool _has_initial_val = false;

protected:
  Value *_llvm_value = nullptr;
  std::shared_ptr<ASTTy> _ty = nullptr;
};

class ASTArgDecl final : public ASTVarDecl {
public:
  ASTArgDecl() = delete;
  ASTArgDecl(Token *token, size_t token_index);

protected:
  size_t nud(Parser *parser) override;
};

/// dummy, all literal types inherit from this class
class ASTLiteral : public ASTNode {
public:
  ASTLiteral() = delete;
  ASTLiteral(ASTType op, int lbp, int rbp, Token *token, size_t token_index);

  bool is_lvalue() const override { return false; }

  bool is_typed() const override { return true; }

  llvm::Value *get_llvm_value(CompilerSession *) const override = 0;
  std::string get_type_name() const override = 0;
  llvm::Type *to_llvm_type(CompilerSession *) const override = 0;
  std::shared_ptr<ASTTy> get_ty() const override = 0;
};

class ASTNumberLiteral final : public ASTLiteral {
public:
  friend class ASTTy;

  friend class ASTMemberAccess;

  friend class Intrinsic;

public:
  explicit ASTNumberLiteral(int value, size_t token_index);
  explicit ASTNumberLiteral(size_t value, size_t token_index);
  explicit ASTNumberLiteral(float value, size_t token_index);
  ASTNumberLiteral(const std::string &str, bool is_float, Token *token, size_t token_index);

  [[nodiscard]] bool is_float() const { return _is_float; }

  Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *compiler_session) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::string to_string(bool print_prefix = true) const override;
  std::shared_ptr<ASTTy> get_ty() const override;

protected:
  size_t nud(Parser *parser) override;

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
  ASTStringLiteral(std::string str, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::shared_ptr<ASTTy> get_ty() const override;

  std::string get_string() const { return _svalue; }

protected:
  size_t nud(Parser *parser) override;

private:
  std::string _svalue;
  llvm::Value *_llvm_value = nullptr;
  llvm::Type *_llvm_type = nullptr;
};

// TODO: set _dominant_idx for infix operators
class ASTAssignment final : public ASTInfixBinaryOp {
public:
  ASTAssignment(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t led(const ASTNodePtr &left, Parser *parser) override;
};

class ASTArithmetic final : public ASTInfixBinaryOp {
public:
  ASTArithmetic() = delete;
  ASTArithmetic(ASTType type, Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t nud(Parser *parser) override; ///< special case for parsing unary plus and minus
};

class ASTCompare final : public ASTInfixBinaryOp {
public:
  ASTCompare() = delete;
  ASTCompare(ASTType type, Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTBinaryNot final : public ASTPrefix {
public:
  ASTBinaryNot() = delete;
  ASTBinaryNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTLogicalNot final : public ASTPrefix {
public:
  ASTLogicalNot() = delete;
  ASTLogicalNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_EXPR_H_
