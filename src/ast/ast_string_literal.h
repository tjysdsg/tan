#ifndef __TAN_SRC_AST_AST_STRING_LITERAL_H__
#define __TAN_SRC_AST_AST_STRING_LITERAL_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

class ASTStringLiteral final : public ASTLiteral {
public:
  ASTStringLiteral() = delete;
  ASTStringLiteral(Token *token, size_t token_index);
  ASTStringLiteral(const std::string &str, size_t token_index);
  llvm::Value *codegen(CompilerSession *) override;
  std::string get_string() const;

protected:
  size_t nud() override;

private:
  std::string _svalue;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STRING_LITERAL_H__ */
