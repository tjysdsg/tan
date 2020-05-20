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
class ASTAmpersand final : public ASTNode, public enable_ptr_from_this<ASTAmpersand> {
public:
  static ASTAmpersandPtr CreateAddressOf(ASTNodePtr n);

public:
  ASTAmpersand() = delete;
  ASTAmpersand(Token *token, size_t token_index);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t led(const ASTNodePtr &left) override;
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_AMPERSAND_H__ */
