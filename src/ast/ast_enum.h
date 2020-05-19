#ifndef __TAN_SRC_AST_AST_ENUM_H__
#define __TAN_SRC_AST_AST_ENUM_H__
#include "src/ast/ast_ty.h"

namespace tanlang {

/**
 * \brief Enum type, a thin wrapper around its underlying type
 * \details Children:
 *  - Underlying type, ASTTy
 * */
class ASTEnum final : public ASTTy {
public:
  ASTEnum() = delete;
  ASTEnum(Token *token, size_t token_index);
  uint64_t get_enum_value(const str &value_name);

protected:
  size_t nud() override;
  llvm::Value *_codegen(CompilerSession *) override;
  llvm::Type *to_llvm_type(CompilerSession *) override;
  llvm::Value *get_llvm_value(CompilerSession *) override;

private:
  umap<str, uint64_t> _enum_values{};
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_ENUM_H__
