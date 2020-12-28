#ifndef __TAN_SRC_AST_AST_FUNC_H__
#define __TAN_SRC_AST_AST_FUNC_H__

#include "src/ast/ast_node.h"

namespace llvm {
class Function;
}

namespace tanlang {

class ASTFunction;
using ASTFunctionPtr = std::shared_ptr<ASTFunction>;

/**
 * \brief Function prototype or definition
 *
 * \details
 * Children:
 *  - Return type, ASTTy
 *  - Arg1, ASTArgDecl
 *  - Arg2, ASTArgDecl
 *  - ...
 * */
class ASTFunction final : public ASTNode {
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

  [[nodiscard]] ASTNodePtr get_ret() const { return _children[0]; }
  [[nodiscard]] ASTNodePtr get_arg(size_t i) const { return _children[i + 1]; }
  [[nodiscard]] size_t get_n_args() const { return _children.size() - 1; }

public:
  llvm::Function *_func = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_FUNC_H__
