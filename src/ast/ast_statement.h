#ifndef TAN_SRC_AST_AST_STATEMENT_H_
#define TAN_SRC_AST_AST_STATEMENT_H_
#include "src/ast/astnode.h"

namespace tanlang {

class ASTProgram final : public ASTNode {
public:
  ASTProgram();
  Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t nud(Parser *parser) override;
};

class ASTStatement final : public ASTNode {
public:
  ASTStatement() = delete;
  ASTStatement(Token *token, size_t token_index);
  ASTStatement(bool is_compound, Token *token, size_t token_index);

protected:
  size_t nud(Parser *parser) override;

public:
  bool _is_compound = false;
};

}

#endif /* TAN_SRC_AST_AST_STATEMENT_H_ */
