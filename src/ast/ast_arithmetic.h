#ifndef __TAN_SRC_AST_AST_ARITHMETIC_H__
#define __TAN_SRC_AST_AST_ARITHMETIC_H__
#include "src/ast/ast_infix_binary_op.h"

namespace tanlang {

class ASTArithmetic final : public ASTInfixBinaryOp {
public:
  ASTArithmetic() = delete;
  ASTArithmetic(ASTType type, Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t nud() override; ///< special case for parsing unary plus and minus
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_ARITHMETIC_H__ */
