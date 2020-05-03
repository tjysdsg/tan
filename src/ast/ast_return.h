#ifndef __TAN_SRC_AST_AST_RETURN_H__
#define __TAN_SRC_AST_AST_RETURN_H__
#include "src/ast/ast_prefix.h"

namespace tanlang {

class ASTReturn final : public ASTPrefix {
public:
  ASTReturn() = delete;
  ASTReturn(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_RETURN_H__ */
