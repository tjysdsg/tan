#ifndef TAN_SRC_AST_AST_ARG_DECL_H
#define TAN_SRC_AST_AST_ARG_DECL_H
#include "base.h"
#include "fwd.h"
#include "ast/ast_base.h"
#include "ast/ast_named.h"
#include "ast/typed.h"
#include "ast/expr.h"

namespace tanlang {

class Decl : public Expr, public ASTNamed {
public:
  bool is_lvalue() override { return true; }
  void set_lvalue(bool) override { TAN_ASSERT(false); }
  [[nodiscard]] vector<ASTBase *> get_children() const override;
  virtual bool is_type_decl() const { return false; }

protected:
  Decl(ASTNodeType type, SourceFile *src, int bp);
};

class VarDecl : public Decl {
protected:
  explicit VarDecl(SourceFile *src);

public:
  static VarDecl *Create(SourceFile *src);
  static VarDecl *Create(SourceFile *src, const str &name, Type *ty);
};

class ArgDecl : public Decl {
protected:
  explicit ArgDecl(SourceFile *src);

public:
  static ArgDecl *Create(SourceFile *src);
  static ArgDecl *Create(SourceFile *src, const str &name, Type *ty);
};

class FunctionType;
class FunctionDecl : public Decl {
protected:
  explicit FunctionDecl(SourceFile *src);

public:
  static FunctionDecl *Create(SourceFile *src);
  static FunctionDecl *Create(SourceFile *src, const str &name, FunctionType *func_type, bool is_external,
                              bool is_public, Stmt *body = nullptr, bool is_intrinsic = false);

public:
  str terminal_token() const override;

  void set_body(Stmt *body);
  [[nodiscard]] Stmt *get_body() const;

  [[nodiscard]] size_t get_n_args() const;
  [[nodiscard]] str get_arg_name(size_t i) const;
  void set_arg_names(const vector<str> &names);
  [[nodiscard]] const vector<ArgDecl *> &get_arg_decls() const;
  void set_arg_decls(const vector<ArgDecl *> &arg_decls);

  [[nodiscard]] bool is_public() const;
  [[nodiscard]] bool is_external() const;
  void set_external(bool is_external);
  void set_public(bool is_public);
  bool is_intrinsic() const;
  void set_is_intrinsic(bool is_intrinsic);

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  bool _is_external = false;
  bool _is_public = false;
  bool _is_intrinsic = false;

  vector<str> _arg_names{};
  vector<ArgDecl *> _arg_decls{};

  Stmt *_body = nullptr;
};

class TypeDecl : public Decl {
public:
  TypeDecl(ASTNodeType node_type, SourceFile *src);
  bool is_type_decl() const override { return true; }
};

class StructDecl : public TypeDecl {
protected:
  explicit StructDecl(SourceFile *src);

public:
  static StructDecl *Create(SourceFile *src);

public:
  const vector<Expr *> &get_member_decls() const;
  void set_member_decls(const vector<Expr *> &member_decls);

  Type *get_struct_member_ty(int i) const;
  vector<Type *> get_member_types() const;

  int get_struct_member_index(const str &name) const;
  void set_member_index(const str &name, int idx);

  Expr *get_member_default_val(int i) const;
  void set_member_default_val(int i, Expr *val);

public:
  vector<ASTBase *> get_children() const override;

  str terminal_token() const override { return "}"; }

private:
  vector<Expr *> _member_decls{};
  umap<int, Expr *> _default_vals{};
  umap<str, int> _member_indices{};
};

} // namespace tanlang

#endif
