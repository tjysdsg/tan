#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include <utility>
#include "src/ast/astnode.h"
#include "token.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
 public:
  ASTIdentifier() = delete;
  explicit ASTIdentifier(std::string name, Token *token) : ASTNode(ASTType::ID, 0, 0, token), _name(std::move(name)) {}
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *parser_context) override;

 public:
  std::string _name{};
};

}

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
