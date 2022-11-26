#ifndef TAN_SRC_AST_AST_ARG_DECL_H
#define TAN_SRC_AST_AST_ARG_DECL_H
#include "base.h"
#include "ast_base.h"
#include "ast_named.h"
#include "typed.h"
#include "expr.h"
#include "fwd.h"

namespace tanlang {

class Decl : public Expr, public ASTNamed {
public:
  bool is_lvalue() override { return true; }
  void set_lvalue(bool) override { TAN_ASSERT(false); }
  [[nodiscard]] vector<ASTBase *> get_children() const override;

protected:
  Decl(ASTNodeType type, SrcLoc loc, int bp);
};

class VarDecl : public Decl {
protected:
  explicit VarDecl(SrcLoc loc);

public:
  static VarDecl *Create(SrcLoc loc);
  static VarDecl *Create(SrcLoc loc, const str &name, Type *ty);
};

class ArgDecl : public Decl {
protected:
  explicit ArgDecl(SrcLoc loc);

public:
  static ArgDecl *Create(SrcLoc loc);
  static ArgDecl *Create(SrcLoc loc, const str &name, Type *ty);
};

// TODO: function type itself
class FunctionDecl : public Decl {
protected:
  explicit FunctionDecl(SrcLoc loc);

public:
  static FunctionDecl *Create(SrcLoc loc);
  static FunctionDecl *Create(SrcLoc loc, const str &name, Type *ret_type, vector<Type *> arg_types, bool is_external,
                              bool is_public, Stmt *body = nullptr);
  static FunctionDecl *GetCallee(ASTContext *ctx, FunctionCall *p);

  [[nodiscard]] Type *get_ret_ty() const;
  void set_ret_type(Type *type);

  void set_body(Stmt *body);
  [[nodiscard]] Stmt *get_body() const;

  [[nodiscard]] size_t get_n_args() const;
  [[nodiscard]] str get_arg_name(size_t i) const;
  [[nodiscard]] Type *get_arg_type(size_t i) const;
  void set_arg_names(const vector<str> &names);
  void set_arg_types(const vector<Type *> &types);
  [[nodiscard]] const vector<ArgDecl *> &get_arg_decls() const;
  void set_arg_decls(const vector<ArgDecl *> &arg_decls);

  [[nodiscard]] bool is_public() const;
  [[nodiscard]] bool is_external() const;
  void set_external(bool is_external);
  void set_public(bool is_public);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  bool _is_external = false;
  bool _is_public = false;

  Type *_ret_type = nullptr;

  vector<str> _arg_names{};
  vector<Type *> _arg_types{};
  vector<ArgDecl *> _arg_decls{};

  Stmt *_body = nullptr;
};

class StructDecl : public Decl {
protected:
  explicit StructDecl(SrcLoc loc);

public:
  static StructDecl *Create(SrcLoc loc);
  const vector<Expr *> &get_member_decls() const;
  void set_member_decls(const vector<Expr *> &member_decls);
  void set_is_forward_decl(bool is_forward_decl);
  bool is_forward_decl() const;
  Type *get_struct_member_ty(size_t i) const;
  size_t get_struct_member_index(const str &name) const;
  void set_member_index(const str &name, size_t idx);

  vector<ASTBase *> get_children() const override;

private:
  vector<Expr *> _member_decls{};
  umap<str, size_t> _member_indices{};
  bool _is_forward_decl = false;
};

class EnumDecl : public Decl {
protected:
  explicit EnumDecl(SrcLoc loc);

public:
  static EnumDecl *Create(SrcLoc loc);
  void set_elements(const vector<Expr *> &elements);
  vector<Expr *> &get_elements();
  int64_t get_value(const str &name) const;
  void set_value(const str &name, int64_t value);
  bool contain_element(const str &name);

private:
  vector<Expr *> _elements{};
  umap<str, int64_t> _element_values{};
};

} // namespace tanlang

#endif
