#ifndef TAN_SRC_AST_AST_CONTROL_FLOW_H_
#define TAN_SRC_AST_AST_CONTROL_FLOW_H_
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTIf final : public ASTNode {
public:
  ASTIf() = delete;
  ASTIf(Token *token, size_t token_index);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t nud() override;

private:
  bool _has_else = false;
};

class ASTElse final : public ASTNode {
public:
  ASTElse() = delete;
  ASTElse(Token *token, size_t token_index);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t nud() override;
};

class ASTBreakContinue final : public ASTNode {
public:
  ASTBreakContinue() = delete;
  ASTBreakContinue(Token *token, size_t token_index);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t nud() override;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_CONTROL_FLOW_H_
