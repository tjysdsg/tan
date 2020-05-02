#ifndef TAN_SRC_AST_AST_FUNC_H_
#define TAN_SRC_AST_AST_FUNC_H_
#include "src/ast/astnode.h"

namespace tanlang {

struct Token;
struct Scope;

class ASTFunction;

using ASTFunctionPtr = std::shared_ptr<ASTFunction>;

/**
 * \brief Function prototype or definition
 *
 * \details
 * Children:
 *  - Return type, ASTTy
 *  - Function name, ASTIdentifier
 *  - Arg1, ASTArgDecl
 *  - Arg2, ASTArgDecl
 *  - ...
 * */
class ASTFunction final : public ASTNode, public std::enable_shared_from_this<ASTFunction> {
public:
  ASTFunction(Token *token, size_t token_index);
  Value *codegen(CompilerSession *) override;
  Value *codegen_prototype(CompilerSession *, bool import = false);
  bool is_named() const override;
  std::string get_name() const override;
  bool is_typed() const override;
  // TODO: std::string get_type_name() const override;
  // TODO: llvm::Type *to_llvm_type(CompilerSession *) const override;
  // TODO: std::shared_ptr<ASTTy> get_ty() const override;

  ASTNodePtr get_ret() const;
  ASTNodePtr get_arg(size_t i) const;
  size_t get_n_args() const;
  Function *get_func() const;
  void set_func(Function *f);

protected:
  size_t nud() override;

private:
  bool _is_external = false;
  bool _is_public = false;
  Function *_func = nullptr;
  std::shared_ptr<Scope> _scope = nullptr;
};

/**
 * \brief Call to a known function (or intrinsic function)
 *
 * \details
 * Children:
 *  - Arg1, ASTNode, typed, valued
 *  - Arg2, ASTNode, typed, valued
 *  - ...
 * */
class ASTFunctionCall final : public ASTNode {
public:
  ASTFunctionCall() = delete;
  ASTFunctionCall(Token *token, size_t token_index);
  Value *codegen(CompilerSession *) override;
  bool is_named() const override;
  std::string get_name() const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  bool is_lvalue() const override;
  bool is_typed() const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::shared_ptr<ASTTy> get_ty() const override;

protected:
  size_t nud() override;
  ASTFunctionPtr get_callee() const;

public:
  std::string _name{};
  Value *_llvm_value = nullptr;
  mutable ASTFunctionPtr _callee = nullptr;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_FUNC_H_ */
