#ifndef __TAN_SRC_AST_AST_EXPR_CPP_AST_PARENTHESIS_H__
#define __TAN_SRC_AST_AST_EXPR_CPP_AST_PARENTHESIS_H__
#include "src/ast/astnode.h"

namespace tanlang {

class ASTParenthesis final : public ASTNode {
public:
  ASTParenthesis() = delete;
  ASTParenthesis(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *compiler_session) override;
  bool is_typed() const override;
  bool is_lvalue() const override;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  llvm::Metadata *to_llvm_meta(CompilerSession *) const override;

protected:
  size_t nud() override;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_EXPR_CPP_AST_PARENTHESIS_H__ */
