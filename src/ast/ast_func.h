#ifndef __TAN_SRC_AST_AST_FUNC_H__
#define __TAN_SRC_AST_AST_FUNC_H__
#include "base.h"
#include "src/ast/ast_type.h"

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

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_FUNC_H__
