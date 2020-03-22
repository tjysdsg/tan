#ifndef __TAN_SRC_AST_AST_DOT_H__
#define __TAN_SRC_AST_AST_DOT_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTDot final : public ASTNode {
public:
  ASTDot() = delete;

  explicit ASTDot(Token *token) : ASTNode(ASTType::MEMBER_ACCESS, op_precedence[ASTType::MEMBER_ACCESS], 0, token) {};
  void led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;
  // Value *codegen(CompilerSession *compiler_session) override;
};

}

#endif /* __TAN_SRC_AST_AST_DOT_H__ */
