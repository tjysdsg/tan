#ifndef __TAN_SRC_AST_AST_LOOP_H__
#define __TAN_SRC_AST_AST_LOOP_H__
#include "src/ast/astnode.h"

namespace tanlang {

enum class ASTLoopType {
  FOR, WHILE,
};

class ASTLoop final : public ASTNode {
public:
  ASTLoop() = delete;

  ASTLoop(Token *token, size_t token_index) : ASTNode(ASTType::LOOP, 0, 0, token, token_index) {}

  Value *codegen(CompilerSession *compiler_session) override;
protected:
  size_t nud() override;
  ASTLoopType _loop_type = ASTLoopType::WHILE;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_LOOP_H__ */
