#ifndef __TAN_SRC_AST_AST_NOT_H__
#define __TAN_SRC_AST_AST_NOT_H__
#include "src/ast/ast_prefix.h"

namespace tanlang {

struct Token;

// TODO: merge these two

class ASTBinaryNot final : public ASTPrefix {
public:
  ASTBinaryNot() = delete;
  ASTBinaryNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

class ASTLogicalNot final : public ASTPrefix {
public:
  ASTLogicalNot() = delete;
  ASTLogicalNot(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_NOT_H__ */
