#include "ast/context.h"
#include "ast/decl.h"

using namespace tanlang;

ASTBase *Context::owner() const { return _owner; }

Context::Context(ASTBase *owner) : _owner(owner) {}

void Context::set_decl(const str &name, Decl *decl) {
  TAN_ASSERT(!name.empty());
  _type_decls[name] = decl;
}

Decl *Context::get_decl(const str &name) const {
  TAN_ASSERT(name != "");
  auto q = _type_decls.find(name);
  if (q != _type_decls.end()) {
    return q->second;
  }
  return nullptr;
}

vector<Decl *> Context::get_decls() const {
  vector<Decl *> ret(_type_decls.size(), nullptr);
  size_t i = 0;
  for (const auto &p : _type_decls)
    ret[i++] = p.second;
  return ret;
}

FunctionDecl *Context::get_func_decl(const str &name) const {
  TAN_ASSERT(name != "");
  auto q = _func_decls.find(name);
  if (q != _func_decls.end()) {
    return q->second;
  }
  return nullptr;
}

void Context::set_function_decl(FunctionDecl *func) {
  str name = func->get_name();
  TAN_ASSERT(!name.empty());
  _func_decls[name] = func;
}

vector<FunctionDecl *> Context::get_func_decls() const {
  vector<FunctionDecl *> ret(_func_decls.size(), nullptr);
  size_t i = 0;
  for (const auto &p : _func_decls)
    ret[i++] = p.second;
  return ret;
}
