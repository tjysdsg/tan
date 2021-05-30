#ifndef TAN_SRC_AST_AST_ARG_DECL_H
#define TAN_SRC_AST_AST_ARG_DECL_H
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"

namespace tanlang {

AST_FWD_DECL(ASTType);

class VarDecl : public ASTBase, public ASTNamed {
public:
  static ptr<VarDecl> Create();
  static ptr<VarDecl> Create(str_view name, const ASTTypePtr &ty);

public:
  VarDecl();
  ASTTypePtr get_type() const { return _type; }
  void set_type(const ASTTypePtr &type) { _type = type; }

private:
  ASTTypePtr _type = nullptr;
  ASTBasePtr _value = nullptr;
};

class ArgDecl : public ASTBase, public ASTNamed {
public:
  static ptr<ArgDecl> Create();

public:
  ArgDecl();
  ASTTypePtr get_type() const { return _type; }
  void set_type(const ASTTypePtr &type) { _type = type; }

private:
  ASTTypePtr _type = nullptr;
};

AST_FWD_DECL(ASTFunction);

class FunctionDecl : public ASTBase, public ASTNamed {
public:
  static ptr<FunctionDecl> Create();
  FunctionDecl();

private:
  ASTFunctionPtr _func = nullptr;
};

AST_FWD_DECL(ASTStruct);
AST_FWD_DECL(Stmt);

class StructDecl : public ASTBase, public ASTNamed {
public:
  static ptr<StructDecl> Create();
  StructDecl();
  void set_member_decls(const vector<StmtPtr> &member_decls);
  void set_is_forward_decl(bool is_forward_decl);

private:
  vector<StmtPtr> _member_decls;
  ASTStructPtr _struct = nullptr;
  bool _is_forward_decl = false;
};

} // namespace tanlang

#endif
