#include "ast/decl.h"
#include "ast/type.h"
#include <algorithm>

using namespace tanlang;

Decl::Decl(ASTNodeType type, SourceFile *src, int bp) : Expr(type, src, bp) {}

vector<ASTBase *> Decl::get_children() const { return {}; }

ArgDecl::ArgDecl(SourceFile *src) : Decl(ASTNodeType::ARG_DECL, src, 0) {}

ArgDecl *ArgDecl::Create(SourceFile *src) { return new ArgDecl(src); }

ArgDecl *ArgDecl::Create(SourceFile *src, const str &name, Type *ty) {
  auto ret = new ArgDecl(src);
  ret->set_name(name);
  ret->set_type(ty);
  return ret;
}

VarDecl::VarDecl(SourceFile *src) : Decl(ASTNodeType::VAR_DECL, src, 0) {}

VarDecl *VarDecl::Create(SourceFile *src) { return new VarDecl(src); }

VarDecl *VarDecl::Create(SourceFile *src, const str &name, Type *ty) {
  auto ret = new VarDecl(src);
  ret->set_name(name);
  ret->set_type(ty);
  return ret;
}

FunctionDecl::FunctionDecl(SourceFile *src) : Decl(ASTNodeType::FUNC_DECL, src, 0) {}

FunctionDecl *FunctionDecl::Create(SourceFile *src) { return new FunctionDecl(src); }

FunctionDecl *FunctionDecl::Create(SourceFile *src, const str &name, FunctionType *func_type, bool is_external,
                                   bool is_public, Stmt *body, bool is_intrinsic) {
  auto ret = new FunctionDecl(src);
  ret->set_name(name);
  if (!body) {
    ret->set_body(body);
  }
  ret->set_type(func_type);
  ret->_is_external = is_external;
  ret->_is_public = is_public;
  ret->_is_intrinsic = is_intrinsic;
  return ret;
}

str FunctionDecl::get_arg_name(size_t i) const { return _arg_names[i]; }

size_t FunctionDecl::get_n_args() const { return _arg_names.size(); }

void FunctionDecl::set_body(Stmt *body) { _body = body; }

void FunctionDecl::set_arg_names(const vector<str> &names) { _arg_names = names; }

bool FunctionDecl::is_public() const { return _is_public; }

bool FunctionDecl::is_external() const { return _is_external; }

Stmt *FunctionDecl::get_body() const { return _body; }

void FunctionDecl::set_external(bool is_external) { _is_external = is_external; }

void FunctionDecl::set_public(bool is_public) { _is_public = is_public; }

bool FunctionDecl::is_intrinsic() const { return _is_intrinsic; }

void FunctionDecl::set_is_intrinsic(bool is_intrinsic) { _is_intrinsic = is_intrinsic; }

const vector<ArgDecl *> &FunctionDecl::get_arg_decls() const { return _arg_decls; }

void FunctionDecl::set_arg_decls(const vector<ArgDecl *> &arg_decls) { _arg_decls = arg_decls; }

vector<ASTBase *> FunctionDecl::get_children() const { return {(ASTBase *)_body}; }

TypeDecl::TypeDecl(ASTNodeType node_type, SourceFile *src) : Decl(node_type, src, 0) {}

StructDecl::StructDecl(SourceFile *src) : TypeDecl(ASTNodeType::STRUCT_DECL, src) {}

StructDecl *StructDecl::Create(SourceFile *src) { return new StructDecl(src); }

const vector<Expr *> &StructDecl::get_member_decls() const { return _member_decls; }

void StructDecl::set_member_decls(const vector<Expr *> &member_decls) { _member_decls = member_decls; }

Type *StructDecl::get_struct_member_ty(int i) const {
  TAN_ASSERT(i >= 0 && i < (int)_member_decls.size());
  return _member_decls[(size_t)i]->get_type();
}

vector<Type *> StructDecl::get_member_types() const {
  auto ret = vector<Type *>(_member_decls.size(), nullptr);
  for (size_t i = 0; i < _member_decls.size(); ++i) {
    ret[i] = _member_decls[i]->get_type();
  }
  return ret;
}

int StructDecl::get_struct_member_index(const str &name) const {
  auto search = _member_indices.find(name);
  if (search == _member_indices.end()) {
    return -1;
  }
  return search->second;
}

void StructDecl::set_member_index(const str &name, int idx) {
  TAN_ASSERT(idx >= 0 && idx < (int)_member_decls.size());
  _member_indices[name] = idx;
}

vector<ASTBase *> StructDecl::get_children() const {
  vector<ASTBase *> ret = {};
  std::for_each(_member_decls.begin(), _member_decls.end(), [&](Expr *e) { ret.push_back(e); });
  return ret;
}

Expr *StructDecl::get_member_default_val(int i) const {
  auto it = _default_vals.find(i);
  if (it == _default_vals.end()) {
    return nullptr;
  }
  return it->second;
}

void StructDecl::set_member_default_val(int i, Expr *val) { _default_vals[i] = val; }
