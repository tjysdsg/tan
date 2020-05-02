#ifndef __TAN_SRC_AST_AST_CAST_H__
#define __TAN_SRC_AST_AST_CAST_H__
#include "src/ast/ast_infix_binary_op.h"

namespace tanlang {

class ASTCast final : public ASTInfixBinaryOp {
public:
  ASTCast() = delete;
  ASTCast(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *compiler_session) override;
  bool is_typed() const override;
  bool is_lvalue() const override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;

protected:
  size_t led(const ASTNodePtr &left) override;
  size_t get_dominant_idx() const override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_CAST_H__ */
