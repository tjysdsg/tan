#ifndef __TAN_SRC_AST_AST_CHAR_LITERAL_H__
#define __TAN_SRC_AST_AST_CHAR_LITERAL_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

class ASTCharLiteral final : public ASTLiteral {
public:
  ASTCharLiteral() = delete;
  ASTCharLiteral(Token *token, size_t token_index);

protected:
  llvm::Value *_codegen(CompilerSession *compiler_session) override;
  size_t nud() override;

private:
  char _c;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_CHAR_LITERAL_H__ */
