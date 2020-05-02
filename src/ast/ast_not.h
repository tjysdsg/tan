#ifndef __TAN_SRC_AST_AST_NOT_H__
#define __TAN_SRC_AST_AST_NOT_H__
#include "src/ast/ast_prefix.h"

namespace tanlang {

class ASTNot final : public ASTPrefix {
public:
  ASTNot() = delete;
  ASTNot(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_NOT_H__ */
