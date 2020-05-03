#ifndef __TAN_SRC_AST_AST_VAR_DECL_H__
#define __TAN_SRC_AST_AST_VAR_DECL_H__
#include "src/ast/ast_node.h"

namespace tanlang {

struct Token;

/**
 * \brief Variable and its declaration
 *
 * \details
 * Children:
 *  - variable name, ASTIdentifier
 *  - type, ASTTy
 *
 * lvalue: true
 * typed: true
 * named: true
 * */
class ASTVarDecl : public ASTNode, public std::enable_shared_from_this<ASTVarDecl> {
public:
  friend class ASTAssignment;
  friend class ASTFunction;

public:
  ASTVarDecl() = delete;
  ASTVarDecl(Token *token, size_t token_index);
  llvm::Value *codegen(CompilerSession *cs) override;
  bool is_typed() const override;
  bool is_named() const override;
  bool is_lvalue() const override;
  void set_ty(std::shared_ptr<ASTTy> ty);
  bool is_type_resolved() const;

protected:
  size_t nud() override;
  size_t _nud();

protected:
  bool _is_type_resolved = false;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_VAR_DECL_H__ */
