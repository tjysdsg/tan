#ifndef __TAN_SRC_AST_AST_CONTROL_FLOW_H__
#define __TAN_SRC_AST_AST_CONTROL_FLOW_H__
#include "src/ast/ast_node.h"

namespace llvm {
class BasicBlock;
}

namespace tanlang {

class ASTIf : public ASTNode {
public:
  ASTIf() = delete;
  ASTIf(ASTNodeType op, int lbp) : ASTNode(op, lbp) {}

public:
  bool _has_else = false;
};

enum class ASTLoopType { FOR, WHILE };

class ASTLoop final : public ASTNode {
public:
  ASTLoop() : ASTNode(ASTNodeType::LOOP, 0) {}

public:
  ASTLoopType _loop_type = ASTLoopType::WHILE;
  llvm::BasicBlock *_loop_start = nullptr;
  llvm::BasicBlock *_loop_end = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_CONTROL_FLOW_H__
