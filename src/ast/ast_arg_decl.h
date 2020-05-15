#ifndef __TAN_SRC_AST_AST_ARG_DECL_H__
#define __TAN_SRC_AST_AST_ARG_DECL_H__
#include "src/ast/ast_var_decl.h"

namespace tanlang {

class ASTArgDecl;
using ASTArgDeclPtr = std::shared_ptr<ASTArgDecl>;

/**
 * \brief Argument declaration
 *
 * \see ASTVarDecl, the only difference, except syntactical ones, is that an argument must have its type specified
 * immediately. In contrast, the type of a variable can be omitted if it can be inferred.
 * */
class ASTArgDecl final : public ASTVarDecl {
public:
  static ASTArgDeclPtr Create(const str &name, ASTTyPtr ty);

public:
  ASTArgDecl() = delete;
  ASTArgDecl(Token *token, size_t token_index);

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_ARG_DECL_H__ */
