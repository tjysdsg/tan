#ifndef __TAN_SRC_AST_AST_NOT_H__
#define __TAN_SRC_AST_AST_NOT_H__
#include "src/ast/ast_prefix.h"

namespace tanlang {

struct Token;

class ASTNot final : public ASTPrefix {
public:
  ASTNot() = delete;
  ASTNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_NOT_H__ */
