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
  size_t get_member_index(std::string name);
  ASTNodePtr get_member(size_t i);
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Value *get_llvm_value(CompilerSession *cs) const override;

protected:
  size_t nud() override;
  mutable llvm::Type *_llvm_type = nullptr;
  std::unordered_map<std::string, size_t> _member_indices{};

private:
  std::string _type_name = "";
  std::vector<std::string> _member_names{};
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STRUCT_H__ */
