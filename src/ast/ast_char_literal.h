#ifndef __TAN_SRC_AST_AST_CHAR_LITERAL_H__
#define __TAN_SRC_AST_AST_CHAR_LITERAL_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

class ASTCharLiteral final : public ASTLiteral {
public:
  ASTCharLiteral() = delete;
  ASTCharLiteral(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::shared_ptr<ASTTy> get_ty() const override;

protected:
  size_t nud() override;

private:
  char _c;
  llvm::Value *_llvm_value = nullptr;
  llvm::Type *_llvm_type = nullptr;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_CHAR_LITERAL_H__ */
