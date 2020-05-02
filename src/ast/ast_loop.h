#ifndef __TAN_SRC_AST_AST_LOOP_H__
#define __TAN_SRC_AST_AST_LOOP_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTLoop final : public ASTNode {
public:
  ASTLoop() = delete;
  ASTLoop(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t nud() override;

private:
  enum class ASTLoopType { FOR, WHILE };
  ASTLoopType _loop_type = ASTLoopType::WHILE;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_LOOP_H__ */
