#ifndef TAN_SRC_AST_AST_FUNC_H_
#define TAN_SRC_AST_AST_FUNC_H_
#include <include/token.h>
#include "src/ast/astnode.h"
#include "src/llvm_include.h"

namespace tanlang {
struct Token;

class ASTFunction final : public ASTNode {
public:
  ASTFunction(Token *token, size_t token_index) : ASTNode(ASTType::FUNC_DECL, 0, 0, token, token_index) {}

  Value *codegen(CompilerSession *compiler_session) override;
  size_t nud(Parser *parser) override;

private:
  bool _is_external = false;
};

class ASTFunctionCall final : public ASTNode {
public:
  ASTFunctionCall() = delete;

  ASTFunctionCall(Token *token, size_t token_index) : ASTNode(ASTType::FUNC_CALL, 0, 0, token, token_index
  ) {
    _name = token->value;
  }

  size_t nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;

public:
  std::string _name{};
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_FUNC_H_ */
