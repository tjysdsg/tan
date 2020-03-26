#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include <utility>
#include "src/ast/astnode.h"
#include "token.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
public:
  ASTIdentifier() = delete;

  ASTIdentifier(std::string name, Token *token) : ASTNode(ASTType::ID, 0, 0, token), _name(std::move(name)) {}

  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
  std::string get_name() const;
  std::string to_string(bool print_prefix = true) const override;

private:
  std::string _name{};
};

}

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
