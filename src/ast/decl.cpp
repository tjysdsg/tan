#include "ast/decl.h"
#include "ast/ast_context.h"
#include "ast/type.h"

using namespace tanlang;

/// \section Decl

Decl::Decl(ASTNodeType type, SrcLoc loc, int bp) : Expr(type, loc, bp) {}

vector<ASTBase *> Decl::get_children() const { return {}; }

/// \section ArgDecl

ArgDecl::ArgDecl(SrcLoc loc) : Decl(ASTNodeType::ARG_DECL, loc, 0) {}

ArgDecl *ArgDecl::Create(SrcLoc loc) { return new ArgDecl(loc); }

ArgDecl *ArgDecl::Create(SrcLoc loc, const str &name, Type *ty) {
  auto ret = new ArgDecl(loc);
  ret->set_name(name);
  ret->set_type(ty);
  return ret;
}

/// \section VarDecl

VarDecl::VarDecl(SrcLoc loc) : Decl(ASTNodeType::VAR_DECL, loc, 0) {}

VarDecl *VarDecl::Create(SrcLoc loc) { return new VarDecl(loc); }

VarDecl *VarDecl::Create(SrcLoc loc, const str &name, Type *ty) {
  auto ret = new VarDecl(loc);
  ret->set_name(name);
  ret->set_type(ty);
  return ret;
}

/// \section FunctionDecl

FunctionDecl::FunctionDecl(SrcLoc loc) : Decl(ASTNodeType::FUNC_DECL, loc, 0) {}

FunctionDecl *FunctionDecl::Create(SrcLoc loc) { return new FunctionDecl(loc); }

FunctionDecl *FunctionDecl::Create(SrcLoc loc, const str &name, FunctionType *func_type, bool is_external,
                                   bool is_public, Stmt *body) {
  auto ret = new FunctionDecl(loc);
  ret->set_name(name);
  if (!body) {
    ret->set_body(body);
  }
  ret->set_type(func_type);
  ret->_is_external = is_external;
  ret->_is_public = is_public;
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

const vector<ArgDecl *> &FunctionDecl::get_arg_decls() const { return _arg_decls; }

void FunctionDecl::set_arg_decls(const vector<ArgDecl *> &arg_decls) { _arg_decls = arg_decls; }

vector<ASTBase *> FunctionDecl::get_children() const { return {(ASTBase *)_body}; }

/// \section StructDecl

StructDecl::StructDecl(SrcLoc loc) : Decl(ASTNodeType::STRUCT_DECL, loc, 0) {}

StructDecl *StructDecl::Create(SrcLoc loc) { return new StructDecl(loc); }

void StructDecl::set_is_forward_decl(bool is_forward_decl) { _is_forward_decl = is_forward_decl; }

bool StructDecl::is_forward_decl() const { return _is_forward_decl; }

const vector<Expr *> &StructDecl::get_member_decls() const { return _member_decls; }

void StructDecl::set_member_decls(const vector<Expr *> &member_decls) { _member_decls = member_decls; }

Type *StructDecl::get_struct_member_ty(size_t i) const {
  TAN_ASSERT(i < _member_decls.size());
  return _member_decls[i]->get_type();
}

size_t StructDecl::get_struct_member_index(const str &name) const {
  auto search = _member_indices.find(name);
  if (search == _member_indices.end()) {
    return (size_t)(-1);
  }
  return search->second;
}

void StructDecl::set_member_index(const str &name, size_t idx) { _member_indices[name] = idx; }

vector<ASTBase *> StructDecl::get_children() const {
  vector<ASTBase *> ret = {};
  std::for_each(_member_decls.begin(), _member_decls.end(), [&](Expr *e) { ret.push_back(e); });
  return ret;
}

EnumDecl::EnumDecl(SrcLoc loc) : Decl(ASTNodeType::ENUM_DECL, loc, 0) {}

EnumDecl *EnumDecl::Create(SrcLoc loc) { return new EnumDecl(loc); }

void EnumDecl::set_elements(const vector<Expr *> &elements) { _elements = elements; }

vector<Expr *> &EnumDecl::get_elements() { return _elements; }

int64_t EnumDecl::get_value(const str &name) const { return _element_values.at(name); }

void EnumDecl::set_value(const str &name, int64_t value) { _element_values[name] = value; }

bool EnumDecl::contain_element(const str &name) { return _element_values.end() != _element_values.find(name); }
