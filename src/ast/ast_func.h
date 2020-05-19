#ifndef TAN_SRC_AST_AST_FUNC_H_
#define TAN_SRC_AST_AST_FUNC_H_
#include "src/ast/ast_node.h"

namespace llvm {
class Function;
}

namespace tanlang {

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
  static ASTFunctionPtr CreateExtern(const str &name, vector<ASTTyPtr> types);

public:
  ASTFunction(Token *token, size_t token_index);
  llvm::Value *codegen_prototype(CompilerSession *, bool import = false);
  bool is_named() override;
  bool is_typed() override;
  // TODO: implement function type

  ASTNodePtr get_ret();
  ASTNodePtr get_arg(size_t i);
  size_t get_n_args();
  llvm::Function *get_func();
  void set_func(llvm::Function *f);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t nud() override;

private:
  bool _is_external = false;
  bool _is_public = false;
  llvm::Function *_func = nullptr;
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
  bool is_named() override;
  bool is_lvalue() override;
  bool is_typed() override;
  void resolve();

public:
  bool _do_resolve = true;

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t nud() override;
  ASTFunctionPtr get_callee();

private:
  mutable ASTFunctionPtr _callee = nullptr;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_FUNC_H_ */
