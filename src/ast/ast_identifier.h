#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include <utility>
#include "src/ast/astnode.h"
#include "token.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
 public:
  ASTIdentifier() = delete;
  // if strict is true, identifier can be a function call
  // FIXME: separate function call from identifier
  ASTIdentifier(std::string name, Token *token, bool strict = false) : ASTNode(ASTType::ID, 0, 0, token),
                                                                       _name(std::move(name)), _strict(strict) {}
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *parser_context) override;

 public:
  std::string _name{};
 private:
  bool _strict;
};

}

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
