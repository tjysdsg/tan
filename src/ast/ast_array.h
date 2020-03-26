#ifndef __TAN_SRC_AST_AST_ARRAY_H__
#define __TAN_SRC_AST_AST_ARRAY_H__
#include "src/ast/astnode.h"
#include "src/ast/interface.h"

namespace tanlang {

/**
 * \brief Array type
 * \details Children are ASTLiteral
 * */
class ASTArrayLiteral : public ASTLiteral {
public:
  ASTArrayLiteral() = delete;

  ASTArrayLiteral(Token *token, size_t token_index) : ASTLiteral(ASTType::ARRAY_LITERAL, 0, 0, token, token_index
  ) {}

  size_t nud(Parser *parser) override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  Value *codegen(CompilerSession *compiler_session) override;
  std::string to_string(bool print_prefix = true) const override;
private:
  llvm::Value *_llvm_value = nullptr;
  llvm::Type *_llvm_type = nullptr;
};

} // namespace tanlang

#endif // __TAN_SRC_AST_AST_ARRAY_H__
