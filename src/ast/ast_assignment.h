#ifndef __TAN_SRC_AST_AST_ASSIGNMENT_H__
#define __TAN_SRC_AST_AST_ASSIGNMENT_H__
#include "src/ast/ast_infix_binary_op.h"

namespace tanlang {

class ASTAssignment final : public ASTInfixBinaryOp {
public:
  ASTAssignment(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
  bool is_lvalue() const override;

protected:
  size_t led(const ASTNodePtr &left) override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_ASSIGNMENT_H__ */
