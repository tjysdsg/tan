#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
public:
  ASTIdentifier() = delete;
  ASTIdentifier(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
  bool is_named() const override;
  str to_string(bool print_prefix = true) const override;
  bool is_lvalue() const override;
  bool is_typed() const override;

protected:
  size_t nud() override;
  ASTNodePtr get_referred(bool strict = true) const;

private:
  mutable ASTNodePtr _referred = nullptr;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
