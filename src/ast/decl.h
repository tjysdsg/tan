#ifndef TAN_SRC_AST_AST_ARG_DECL_H
#define TAN_SRC_AST_AST_ARG_DECL_H
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"
#include "src/ast/expr.h"

namespace tanlang {

AST_FWD_DECL(ASTType);
AST_FWD_DECL(Decl);
AST_FWD_DECL(CompoundStmt);
AST_FWD_DECL(FunctionDecl);
AST_FWD_DECL(Stmt);
AST_FWD_DECL(Expr);

class Decl : public Expr, public ASTNamed {
public:
  Decl(ASTNodeType type, int lbp);
};

class VarDecl : public Decl {
public:
  static ptr<VarDecl> Create();
  static ptr<VarDecl> Create(const str &name, const ASTTypePtr &ty);

public:
  VarDecl();

private:
  ASTBasePtr _value = nullptr;
};

class ArgDecl : public Decl {
public:
  static ptr<ArgDecl> Create();
  static ptr<ArgDecl> Create(const str &name, const ASTTypePtr &ty);

public:
  ArgDecl();
};

// TODO: function type itself
class FunctionDecl : public Decl {
public:
  static FunctionDeclPtr Create();
  static FunctionDeclPtr Create(const str &name,
      const ASTTypePtr &ret_type,
      vector<ASTTypePtr> arg_types,
      bool is_external,
      bool is_public);
  static FunctionDeclPtr GetCallee(CompilerSession *cs, const str &name, const vector<ExprPtr> &args);
  FunctionDecl();

  [[nodiscard]] ASTTypePtr get_ret_ty() const;
  void set_ret_type(ASTTypePtr type);

  void set_body(StmtPtr body);
  StmtPtr get_body() const;

  [[nodiscard]] size_t get_n_args() const;
  [[nodiscard]] str get_arg_name(size_t i) const;
  [[nodiscard]] ASTTypePtr get_arg_type(size_t i) const;
  void set_arg_names(const vector<str> &names);
  void set_arg_types(const vector<ASTTypePtr> &types);
  const vector<ptr<ArgDecl>> &get_arg_decls() const;
  void set_arg_decls(const vector<ptr<ArgDecl>> &arg_decls);

  bool is_public() const;
  bool is_external() const;
  void set_external(bool is_external);
  void set_public(bool is_public);

private:
  bool _is_external = false;
  bool _is_public = false;

  ASTTypePtr _ret_type = nullptr;

  vector<str> _arg_names{};
  vector<ASTTypePtr> _arg_types{};
  vector<ptr<ArgDecl>> _arg_decls{};

  StmtPtr _body = nullptr;
};

class StructDecl : public Decl {
public:
  static ptr<StructDecl> Create();
  StructDecl();
  const vector<ExprPtr> &get_member_decls() const;
  void set_member_decls(const vector<ExprPtr> &member_decls);
  void set_is_forward_decl(bool is_forward_decl);
  bool is_forward_decl() const;
  ASTTypePtr get_struct_member_ty(size_t i) const;
  size_t get_struct_member_index(const str &name) const;
  void set_member_index(const str &name, size_t idx);

private:
  vector<ExprPtr> _member_decls{};
  umap<str, size_t> _member_indices{};
  bool _is_forward_decl = false;
};

} // namespace tanlang

#endif
