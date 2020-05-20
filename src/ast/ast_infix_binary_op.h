#ifndef __TAN_SRC_AST_AST_INFIX_BINARY_OP_H__
#define __TAN_SRC_AST_AST_INFIX_BINARY_OP_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTInfixBinaryOp : public ASTNode {
public:
  ASTInfixBinaryOp() = delete;
  ASTInfixBinaryOp(Token *token, size_t token_index);

protected:
  virtual size_t get_dominant_idx();
  size_t _dominant_idx = 0;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_INFIX_BINARY_OP_H__ */
