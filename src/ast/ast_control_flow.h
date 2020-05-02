#ifndef TAN_SRC_AST_AST_CONTROL_FLOW_H_
#define TAN_SRC_AST_AST_CONTROL_FLOW_H_
#include "src/ast/astnode.h"

namespace tanlang {

class ASTIf final : public ASTNode {
public:
  ASTIf() = delete;
  ASTIf(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;

protected:
  size_t nud() override;

private:
  bool _has_else = false;
};

class ASTElse final : public ASTNode {
public:
  ASTElse() = delete;
  ASTElse(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_CONTROL_FLOW_H_
