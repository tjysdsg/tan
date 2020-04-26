#ifndef __TAN_SRC_AST_AST_LITERAL_H__
#define __TAN_SRC_AST_AST_LITERAL_H__
#include "src/ast/astnode.h"

namespace tanlang {

struct Token;

/// dummy, all literal types inherit from this class
class ASTLiteral : public ASTNode {
public:
  ASTLiteral() = delete;
  ASTLiteral(ASTType op, int lbp, int rbp, Token *token, size_t token_index);
  bool is_lvalue() const override;
  bool is_typed() const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override = 0;
  std::string get_type_name() const override = 0;
  llvm::Type *to_llvm_type(CompilerSession *) const override = 0;
  std::shared_ptr<ASTTy> get_ty() const override = 0;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_LITERAL_H__ */
