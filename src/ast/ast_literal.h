#ifndef __TAN_SRC_AST_AST_LITERAL_H__
#define __TAN_SRC_AST_AST_LITERAL_H__
#include "src/ast/ast_node.h"

namespace tanlang {

/// dummy, all literal types inherit from this class
class ASTLiteral : public ASTNode {
public:
  ASTLiteral() = delete;
  ASTLiteral(ASTType op, int lbp, int rbp, Token *token, size_t token_index);
  bool is_typed() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_LITERAL_H__ */
