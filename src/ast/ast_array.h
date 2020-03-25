#ifndef __TAN_SRC_AST_AST_ARRAY_H__
#define __TAN_SRC_AST_AST_ARRAY_H__
#include "src/ast/astnode.h"

namespace tanlang {

/**
 * \brief Array type
 * \details Children are ASTLiteral
 * */
class ASTArrayLiteral : public ASTLiteral {
public:
  ASTArrayLiteral() = delete;

  explicit ASTArrayLiteral(Token *token) : ASTLiteral(ASTType::ARRAY_LITERAL, 0, 0, token) {}

  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
};

} // namespace tanlang

#endif // __TAN_SRC_AST_AST_ARRAY_H__
