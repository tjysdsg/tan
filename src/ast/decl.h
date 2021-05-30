#ifndef TAN_SRC_AST_AST_ARG_DECL_H
#define TAN_SRC_AST_AST_ARG_DECL_H
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"

namespace tanlang {

AST_FWD_DECL(ASTType);
AST_FWD_DECL(Decl);

class Decl : public ASTBase, public ASTNamed, public Typed {
public:
  Decl(ASTNodeType type, int lbp);
};

class VarDecl : public Decl {
public:
  static ptr<VarDecl> Create();
  static ptr<VarDecl> Create(str_view name, const ASTTypePtr &ty);

public:
  VarDecl();

private:
  ASTBasePtr _value = nullptr;
};

class ArgDecl : public Decl {
public:
  static ptr<ArgDecl> Create();
  static ptr<ArgDecl> Create(str_view name, const ASTTypePtr &ty);

public:
  ArgDecl();
};

AST_FWD_DECL(FunctionDecl);
AST_FWD_DECL(Stmt);
AST_FWD_DECL(Expr);

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
  [[nodiscard]] str get_arg_name(size_t i) const;
  [[nodiscard]] ASTTypePtr get_arg_type(size_t i) const;
  [[nodiscard]] size_t get_n_args() const;
  void set_body(StmtPtr body);
  StmtPtr get_body() const;
  void set_ret_type(ASTTypePtr type);
  void set_arg_names(const vector<str> &names);
  void set_arg_types(const vector<ASTTypePtr> &types);
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
  StmtPtr _body = nullptr;
};

AST_FWD_DECL(ASTStruct);

class StructDecl : public Decl {
public:
  static ptr<StructDecl> Create();
  StructDecl();
  const vector<DeclPtr> &get_member_decls() const;
  void set_member_decls(const vector<DeclPtr> &member_decls);
  void set_is_forward_decl(bool is_forward_decl);
  bool is_forward_decl() const;

private:
  vector<DeclPtr> _member_decls;

private:
  ASTStructPtr _struct = nullptr;
  bool _is_forward_decl = false;
};

} // namespace tanlang

#endif
