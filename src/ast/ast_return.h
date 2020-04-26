#ifndef __TAN_SRC_AST_AST_RETURN_H__
#define __TAN_SRC_AST_AST_RETURN_H__
#include "src/ast/ast_prefix.h"

namespace tanlang {

struct Token;

class ASTReturn final : public ASTPrefix {
public:
  ASTReturn() = delete;
  ASTReturn(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Metadata *to_llvm_meta(CompilerSession *) const override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_RETURN_H__ */
