#ifndef TAN_SRC_AST_AST_CONTROL_FLOW_H_
#define TAN_SRC_AST_AST_CONTROL_FLOW_H_
#include "src/ast/astnode.h"
#include "token.h"

namespace tanlang {
class Parser;

class CompilerSession;

class ASTIf final : public ASTNode {
public:
  ASTIf() = delete;

  ASTIf(Token *token, size_t token_index) : ASTNode(ASTType::IF, op_precedence[ASTType::IF], 0, token, token_index) {}

  Value *codegen(CompilerSession *compiler_session) override;
protected:
  size_t nud(Parser *parser) override;

public:
  bool _has_else = false;
};

class ASTElse final : public ASTNode {
public:
  ASTElse() = delete;

  explicit ASTElse(Token *token, size_t token_index) : ASTNode(ASTType::ELSE,
      op_precedence[ASTType::ELSE],
      0,
      token,
      token_index) {}

  Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t nud(Parser *parser) override;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_CONTROL_FLOW_H_
