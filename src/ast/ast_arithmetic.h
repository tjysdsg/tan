#ifndef __TAN_SRC_AST_AST_ARITHMETIC_H__
#define __TAN_SRC_AST_AST_ARITHMETIC_H__
#include "src/ast/ast_infix_binary_op.h"

namespace tanlang {

/**
 * \brief Arithmetic operators, + - * /
 *
 * \details
 * Children:
 *  - Left-hand operand, ASTNode, typed, valued
 *  - Right-hand operand, ASTNode, typed, valued
 *
 * lvalue: false
 * typed: true
 * */
class ASTArithmetic final : public ASTInfixBinaryOp {
public:
  ASTArithmetic() = delete;
  ASTArithmetic(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *cs) override;

protected:
  size_t nud() override; /// special case for parsing unary plus and minus
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_ARITHMETIC_H__ */
