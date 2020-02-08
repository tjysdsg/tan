#ifndef TAN_SRC_AST_AST_FUNC_H_
#define TAN_SRC_AST_AST_FUNC_H_
#include "src/ast/astnode.h"
#include "src/llvm_include.h"

namespace tanlang {
struct Token;

class ASTFunction : public ASTNode {
 public:
  explicit ASTFunction(Token *token) : ASTNode(ASTType::FUNC_DECL, 0, 0, token) {}
  Value *codegen(CompilerSession *compiler_session) override;
  void nud(Parser *parser) override;
};

class ASTFunctionCall final : public ASTNode {
 public:
  ASTFunctionCall() = delete;
  ASTFunctionCall(std::string name, Token *token) : ASTNode(ASTType::FUNC_CALL, 0, 0, token),
                                                    _name(std::move(name)) {}
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;

 public:
  std::string _name{};
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_FUNC_H_ */
