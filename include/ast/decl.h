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

class FunctionType;
class FunctionDecl : public Decl {
protected:
  explicit FunctionDecl(SrcLoc loc);

public:
  static FunctionDecl *Create(SrcLoc loc);
  static FunctionDecl *Create(SrcLoc loc, const str &name, FunctionType *func_type, bool is_external, bool is_public,
                              Stmt *body = nullptr);

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

  [[nodiscard]] vector<ASTBase *> get_children() const override;

private:
  bool _is_external = false;
  bool _is_public = false;

  vector<str> _arg_names{};
  vector<ArgDecl *> _arg_decls{};

  Stmt *_body = nullptr;
};

class TypeDecl : public Decl {
public:
  TypeDecl(ASTNodeType node_type, SrcLoc loc);
  bool is_type_decl() const override { return true; }
};

class StructDecl : public TypeDecl {
protected:
  explicit StructDecl(SrcLoc loc);

public:
  static StructDecl *Create(SrcLoc loc);
  const vector<Expr *> &get_member_decls() const;
  void set_member_decls(const vector<Expr *> &member_decls);
  Type *get_struct_member_ty(size_t i) const;
  size_t get_struct_member_index(const str &name) const;
  void set_member_index(const str &name, size_t idx);
  vector<ASTBase *> get_children() const override;

private:
  vector<Expr *> _member_decls{};
  umap<str, size_t> _member_indices{};
};

} // namespace tanlang

#endif
