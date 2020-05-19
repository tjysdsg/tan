#ifndef __TAN_SRC_AST_AST_ENUM_H__
#define __TAN_SRC_AST_AST_ENUM_H__
#include "src/ast/ast_ty.h"

namespace tanlang {

/**
 * \brief Enum type
 * \details No children:
 * */
class ASTEnum final : public ASTTy {
public:
  ASTEnum() = delete;
  ASTEnum(Token *token, size_t token_index);

protected:
  size_t nud() override;
  llvm::Value *_codegen(CompilerSession *) override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;

private:
  umap<str, uint64_t> _enum_values{};
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_ENUM_H__
