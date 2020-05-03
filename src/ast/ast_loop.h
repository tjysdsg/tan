#ifndef __TAN_SRC_AST_AST_LOOP_H__
#define __TAN_SRC_AST_AST_LOOP_H__
#include "src/ast/ast_node.h"

namespace llvm {
class BasicBlock;
}

namespace tanlang {

class ASTLoop final : public ASTNode, public std::enable_shared_from_this<ASTLoop> {
public:
  ASTLoop() = delete;
  ASTLoop(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
  llvm::BasicBlock *get_loop_end() const;
  llvm::BasicBlock *get_loop_start() const;

protected:
  size_t nud() override;

private:
  enum class ASTLoopType { FOR, WHILE };
  ASTLoopType _loop_type = ASTLoopType::WHILE;
  llvm::BasicBlock *_loop_start = nullptr;
  llvm::BasicBlock *_loop_end = nullptr;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_LOOP_H__ */
