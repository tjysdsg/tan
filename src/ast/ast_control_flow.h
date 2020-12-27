#ifndef __TAN_SRC_AST_AST_CONTROL_FLOW_H__
#define __TAN_SRC_AST_AST_CONTROL_FLOW_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTIf : public ASTNode {
public:
  ASTIf() = delete;
  ASTIf(ASTType op, int lbp) : ASTNode(op, lbp) {}

public:
  bool _has_else = false;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_CONTROL_FLOW_H__
