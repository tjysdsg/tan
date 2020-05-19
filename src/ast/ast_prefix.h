#ifndef __TAN_SRC_AST_AST_PREFIX_H__
#define __TAN_SRC_AST_AST_PREFIX_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTPrefix : public ASTNode {
public:
  ASTPrefix() = delete;
  ASTPrefix(Token *token, size_t token_index);
  bool is_typed() override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_PREFIX_H__ */
