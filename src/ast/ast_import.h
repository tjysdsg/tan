#ifndef __TAN_SRC_AST_AST_IMPORT_H__
#define __TAN_SRC_AST_AST_IMPORT_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTFunction;

class ASTImport : public ASTNode {
public:
  ASTImport() = delete;
  ASTImport(Token *token, size_t token_index);
  str to_string(bool print_prefix = true) const override;

protected:
  llvm::Value *_codegen(CompilerSession *cm) override;
  size_t nud() override;
  str _file = "";
  vector<std::shared_ptr<ASTFunction>> _imported_functions{};
};

} // namespace tanlang

#endif // __TAN_SRC_AST_AST_IMPORT_H__
