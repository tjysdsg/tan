#ifndef __TAN_SRC_AST_AST_ARG_DECL_H__
#define __TAN_SRC_AST_AST_ARG_DECL_H__
#include "src/ast/ast_var_decl.h"

namespace tanlang {

struct Token;

class ASTArgDecl final : public ASTVarDecl {
public:
  ASTArgDecl() = delete;
  ASTArgDecl(Token *token, size_t token_index);

protected:
  size_t nud(Parser *parser) override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_ARG_DECL_H__ */
