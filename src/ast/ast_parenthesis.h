#ifndef __TAN_SRC_AST_AST_EXPR_CPP_AST_PARENTHESIS_H__
#define __TAN_SRC_AST_AST_EXPR_CPP_AST_PARENTHESIS_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTParenthesis final : public ASTNode {
public:
  ASTParenthesis() = delete;
  ASTParenthesis(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
  bool is_typed() const override;
  bool is_lvalue() const override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_EXPR_CPP_AST_PARENTHESIS_H__ */
