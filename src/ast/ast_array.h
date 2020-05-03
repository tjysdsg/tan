#ifndef __TAN_SRC_AST_AST_ARRAY_H__
#define __TAN_SRC_AST_AST_ARRAY_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

/**
 * \brief Array type
 * \details
 * Children:
 *  - element1, ASTLiteral
 *  - element2, ASTLiteral
 *  - ...
 *
 * lvalue: false
 * typed: true
 * */
class ASTArrayLiteral : public ASTLiteral {
public:
  ASTArrayLiteral() = delete;
  ASTArrayLiteral(Token *token, size_t token_index);
  llvm::Type *get_element_llvm_type(CompilerSession *) const;
  size_t get_n_elements() const;
  llvm::Value *codegen(CompilerSession *compiler_session) override;
  std::string to_string(bool print_prefix = true) const override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif // __TAN_SRC_AST_AST_ARRAY_H__
