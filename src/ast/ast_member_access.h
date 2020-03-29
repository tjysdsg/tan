#ifndef __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#define __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTMemberAccess final : public ASTNode {
public:
  ASTMemberAccess() = delete;

  ASTMemberAccess(Token *token, size_t token_index) : ASTNode(ASTType::MEMBER_ACCESS,
                                                              op_precedence[ASTType::MEMBER_ACCESS],
                                                              0,
                                                              token,
                                                              token_index
  ) {};
  Value *codegen(CompilerSession *compiler_session) override;
protected:
  size_t led(const std::shared_ptr<ASTNode> &left, Parser *parser) override;

private:
  bool _is_bracket = false;
};

}

#endif /*__TAN_SRC_AST_AST_MEMBER_ACCESS_H__*/
