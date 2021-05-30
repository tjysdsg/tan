#include "src/ast/ast_node.h"
#include "src/ast/ast_ty.h"
#include "parser.h"
#include "token.h"
#include <iostream>

using namespace tanlang;

ASTNode::ASTNode(ASTNodeType op, int lbp) {
  set_node_type(op);
  set_lbp(lbp);
}
