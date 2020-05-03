#ifndef TAN_SRC_AST_AST_STATEMENT_H_
#define TAN_SRC_AST_AST_STATEMENT_H_
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTStatement final : public ASTNode {
public:
  ASTStatement() = delete;
  ASTStatement(Token *token, size_t token_index);
  ASTStatement(bool is_compound, Token *token, size_t token_index);

protected:
  size_t nud() override;

public:
  bool _is_compound = false;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_STATEMENT_H_ */
