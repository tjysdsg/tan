#ifndef __TAN_SRC_AST_AST_AMPERSAND_H__
#define __TAN_SRC_AST_AST_AMPERSAND_H__
#include "src/ast/astnode.h"

namespace tanlang {

/**
 * Class that delegates two types of ASTNode
 * - TODO: Binary and
 * - address_of
 * */
class ASTAmpersand final : public ASTNode {
public:
  ASTAmpersand() = delete;
  ASTAmpersand(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  bool is_typed() const override;
  bool is_lvalue() const override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;

protected:
  size_t led(const ASTNodePtr &left) override;
  size_t nud() override;

private:
  std::shared_ptr<ASTTy> _ty = nullptr;
  Value *_llvm_value = nullptr;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_AMPERSAND_H__ */
