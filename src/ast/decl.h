#ifndef TAN_SRC_AST_AST_ARG_DECL_H
#define TAN_SRC_AST_AST_ARG_DECL_H
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_named.h"

namespace tanlang {

AST_FWD_DECL(ASTType);

class VarDecl : public ASTBase, public ASTNamed {
public:
  ptr<VarDecl> Create();
  ptr<VarDecl> Create(str_view name, const ASTTypePtr &ty);

private:
  VarDecl();

private:
  optional<ASTTypePtr> _type = nullptr;
  optional<ASTBasePtr> _value = nullptr;
};

class ArgDecl : public ASTBase, public ASTNamed {
public:
  ptr<ArgDecl> Create();

private:
  ArgDecl();

private:
  ASTTypePtr _type = nullptr;
};

AST_FWD_DECL(ASTFunction);

class FunctionDecl : public ASTBase, public ASTNamed {
public:
  ptr<FunctionDecl> Create();

private:
  FunctionDecl();

private:
  ASTFunctionPtr _func = nullptr;
};

AST_FWD_DECL(ASTStruct);

class StructDecl : public ASTBase, public ASTNamed {
public:
  ptr<StructDecl> Create();

private:
  StructDecl();

private:
  ASTStructPtr _struct = nullptr;
};

} // namespace tanlang

#endif
