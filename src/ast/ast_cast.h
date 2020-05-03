#ifndef __TAN_SRC_AST_AST_CAST_H__
#define __TAN_SRC_AST_AST_CAST_H__
#include "src/ast/ast_infix_binary_op.h"

namespace tanlang {

/**
 * \brief Explicit cast
 * \details
 * Children:
 *  - Left-hand operand, ASTNode, valued, typed
 *  - Destination type, ASTTy
 * */
class ASTCast final : public ASTInfixBinaryOp {
public:
  ASTCast() = delete;
  ASTCast(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
  bool is_typed() const override;
  bool is_lvalue() const override;

protected:
  size_t led(const ASTNodePtr &left) override;
  size_t get_dominant_idx() const override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_CAST_H__ */
