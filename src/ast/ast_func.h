#ifndef __TAN_SRC_AST_AST_FUNC_H__
#define __TAN_SRC_AST_AST_FUNC_H__
#include "base.h"
#include "src/ast/ast_ty.h"

namespace llvm {
class Function;
}

namespace tanlang {

AST_FWD_DECL(ASTFunction);

/**
 * \brief Function prototype or definition
 * */
class ASTFunction {
public:
  static ASTFunctionPtr CreateExtern(const str &name, vector<ASTTypePtr> types);
  static ASTFunctionPtr GetCallee(CompilerSession *cs, const str &name, const vector<ASTBasePtr> &args);

public:
  ASTFunction() : {
    _is_external = false;
    _is_public = false;
  };

  [[nodiscard]] ASTTypePtr get_ret_ty() const;
  [[nodiscard]] ASTBasePtr get_arg(size_t i) const;
  [[nodiscard]] size_t get_n_args() const;

public:
  llvm::Function *_func = nullptr;
  bool _is_external = false;
  bool _is_public = false;
};

class ASTFunctionCall : public ASTNode {
public:
  ASTFunctionCall() : ASTNode(ASTNodeType::FUNC_CALL, 0) {
    _is_typed = true;
    _is_valued = true;
    _is_named = true;
  }
  ASTFunctionPtr _callee = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_FUNC_H__
