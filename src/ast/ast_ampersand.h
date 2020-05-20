#ifndef __TAN_SRC_AST_AST_AMPERSAND_H__
#define __TAN_SRC_AST_AST_AMPERSAND_H__
#include "src/ast/ast_node.h"

namespace tanlang {
class ASTAmpersand;
using ASTAmpersandPtr = std::shared_ptr<ASTAmpersand>;

/**
 * Class that delegates two types of ASTNode:
 * - TODO: Binary and
 * - address_of
 *
 * \details
 * Address of:
 *  - Children: right-hand operand, ASTNode
 *  - lvalue: false
 *  - typed: true
 * */

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_AMPERSAND_H__ */
