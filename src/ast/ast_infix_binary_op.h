#ifndef __TAN_SRC_AST_AST_INFIX_BINARY_OP_H__
#define __TAN_SRC_AST_AST_INFIX_BINARY_OP_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTInfixBinaryOp : public ASTNode {
public:
  ASTInfixBinaryOp() = delete;
  ASTInfixBinaryOp(Token *token, size_t token_index);
  bool is_typed() const override;
  bool is_lvalue() const override;
  llvm::Metadata *to_llvm_meta(CompilerSession *) const override;

protected:
  virtual size_t get_dominant_idx() const;
  size_t led(const ASTNodePtr &left) override;
  size_t _dominant_idx = 0;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_INFIX_BINARY_OP_H__ */
