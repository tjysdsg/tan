#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include <utility>
#include "src/ast/astnode.h"
#include "token.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
public:
  ASTIdentifier() = delete;

  ASTIdentifier(Token *token, size_t token_index) : ASTNode(ASTType::ID, 0, 0, token, token_index) {
    _name = token->value;
  }

  bool is_named() const override { return true; }

  Value *codegen(CompilerSession *compiler_session) override;

  std::string get_name() const override;
  std::string to_string(bool print_prefix = true) const override;

  llvm::Value *get_llvm_value(CompilerSession *) const override { return _llvm_value; };

  bool is_lvalue() const override { return true; }

protected:
  size_t nud(Parser *parser) override;
private:
  std::string _name{};
  llvm::Value *_llvm_value = nullptr;
};

}

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
