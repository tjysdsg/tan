#ifndef __TAN_SRC_AST_AST_FUNC_H__
#define __TAN_SRC_AST_AST_FUNC_H__

#include "src/ast/ast_node.h"

namespace llvm {
class Function;
}

namespace tanlang {

AST_FWD_DECL(ASTFunction);

/**
 * \brief Function prototype or definition
 *
 * \details
 * Children:
 *  - Return type, ASTTy
 *  - Arg1, ASTArgDecl
 *  - Arg2, ASTArgDecl
 *  - ...
 *  - [Optional] Function body, ASTNode
 * */
class ASTFunction : public ASTNode {
public:
  static ASTFunctionPtr CreateExtern(const str &name, vector<ASTTyPtr> types);
  static ASTFunctionPtr GetCallee(CompilerSession *cs, const str &name, const vector<ASTNodePtr> &args);

public:
  ASTFunction() : ASTNode(ASTType::FUNC_DECL, 0) {
    _is_named = true;
    _is_typed = true;
    _is_external = false;
    _is_public = false;
  };

  [[nodiscard]] ASTTyPtr get_ret_ty() const;
  [[nodiscard]] ASTNodePtr get_arg(size_t i) const;
  [[nodiscard]] size_t get_n_args() const;

public:
  llvm::Function *_func = nullptr;
  bool _is_external = false;
  bool _is_public = false;
};

class ASTFunctionCall : public ASTNode {
public:
  ASTFunctionCall() : ASTNode(ASTType::FUNC_CALL, 0) {
    _is_typed = true;
    _is_valued = true;
    _is_named = true;
  }
  ASTFunctionPtr _callee = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_FUNC_H__
