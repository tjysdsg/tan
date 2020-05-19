#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
public:
  ASTIdentifier() = delete;
  ASTIdentifier(Token *token, size_t token_index);
  bool is_named() override;
  str to_string(bool print_prefix = true) override;
  bool is_lvalue() override;
  bool is_typed() override;
  ASTNodePtr get_referred(bool strict = true);

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t nud() override;

private:
  mutable ASTNodePtr _referred = nullptr;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
