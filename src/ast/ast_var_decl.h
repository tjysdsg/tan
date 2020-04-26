#ifndef __TAN_SRC_AST_AST_VAR_DECL_H__
#define __TAN_SRC_AST_AST_VAR_DECL_H__
#include "src/ast/astnode.h"

namespace tanlang {

struct Token;

class ASTVarDecl : public ASTNode, public std::enable_shared_from_this<ASTVarDecl> {
public:
  friend class ASTAssignment;

  friend class ASTFunction;

  ASTVarDecl() = delete;
  ASTVarDecl(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  bool is_typed() const override;
  bool is_named() const override;
  std::string get_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *compiler_session) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  bool is_lvalue() const override;

protected:
  size_t nud(Parser *parser) override;
  size_t _nud(Parser *parser);

public:
  bool _has_initial_val = false;

protected:
  Value *_llvm_value = nullptr;
  std::shared_ptr<ASTTy> _ty = nullptr;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_VAR_DECL_H__ */
