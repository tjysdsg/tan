#include "src/ast/decl.h"

using namespace tanlang;

/// \section ArgDecl

ArgDecl::ArgDecl() : ASTBase(ASTNodeType::ARG_DECL, 0) {}

ptr<ArgDecl> ArgDecl::Create() { return make_ptr<ArgDecl>(); }

/// \section VarDecl

VarDecl::VarDecl() : ASTBase(ASTNodeType::VAR_DECL, 0) {}

ptr<VarDecl> VarDecl::Create() { return make_ptr<VarDecl>(); }

ptr<VarDecl> VarDecl::Create(str_view name, const ASTTypePtr &ty) {
  auto ret = make_ptr<VarDecl>();
  ret->set_name(name);
  ret->_type = ty;
  return ret;
}

/// \section FunctionDecl

FunctionDecl::FunctionDecl() : ASTBase(ASTNodeType::FUNC_DECL, 0) {}

ptr<FunctionDecl> FunctionDecl::Create() {
  return make_ptr<FunctionDecl>();
}

/// \section StructDecl

StructDecl::StructDecl() : ASTBase(ASTNodeType::STRUCT_DECL, 0) {}

ptr<StructDecl> StructDecl::Create() {
  return make_ptr<StructDecl>();
}

void StructDecl::set_member_decls(const vector<StmtPtr> &member_decls) { _member_decls = member_decls; }

void StructDecl::set_is_forward_decl(bool is_forward_decl) {
  _is_forward_decl = is_forward_decl;
}
