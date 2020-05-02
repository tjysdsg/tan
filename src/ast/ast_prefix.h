#ifndef __TAN_SRC_AST_AST_PREFIX_H__
#define __TAN_SRC_AST_AST_PREFIX_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTPrefix : public ASTNode {
public:
  ASTPrefix() = delete;
  ASTPrefix(Token *token, size_t token_index);
  bool is_typed() const override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_PREFIX_H__ */
