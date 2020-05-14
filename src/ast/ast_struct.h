#ifndef __TAN_SRC_AST_AST_STRUCT_H__
#define __TAN_SRC_AST_AST_STRUCT_H__
#include "src/ast/ast_ty.h"
#include <unordered_map>

namespace tanlang {

/**
 * \brief Struct type
 * \details nud() function also handles struct declaration.
 * Children:
 *  - ID: the name of the struct
 *  - ASTTy1
 *  - ASTTy2
 *  - ...
 * */
class ASTStruct final : public ASTTy {
public:
  ASTStruct() = delete;
  ASTStruct(Token *token, size_t token_index);
  size_t get_member_index(str name);
  ASTNodePtr get_member(size_t i);
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;

protected:
  size_t nud() override;

private:
  std::unordered_map<str, size_t> _member_indices{};
  std::vector<str> _member_names{};
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STRUCT_H__ */
