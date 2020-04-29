#ifndef __TAN_SRC_AST_AST_IMPORT_H__
#define __TAN_SRC_AST_AST_IMPORT_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTImport : public ASTNode {
public:
  ASTImport() = delete;
  ASTImport(Token *token, size_t token_index);
  Value *codegen(CompilerSession *cm) override;
  std::string to_string(bool print_prefix = true) const override;
protected:
  size_t nud() override;
  std::string _file = "";
};

} // namespace tanlang

#endif // __TAN_SRC_AST_AST_IMPORT_H__
