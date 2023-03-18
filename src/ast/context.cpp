#include "ast/context.h"
#include "ast/function_table.h"

using namespace tanlang;

Context::Context(ASTBase *owner) : _owner(owner) {}

Decl *Context::get_decl(const str &name) {
  TAN_ASSERT(name != "");
  auto q = _type_decls.find(name);
  if (q != _type_decls.end()) {
    return q->second;
  }
  return nullptr;
}

void Context::set_decl(const str &name, Decl *decl) {
  TAN_ASSERT(!name.empty());
  _type_decls[name] = decl;
}

void Context::add_function_decl(FunctionDecl *func) { _function_table.set(func); }

vector<Decl *> Context::get_decls() {
  vector<Decl *> ret(_type_decls.size(), nullptr);
  size_t i = 0;
  for (const auto &p : _type_decls)
    ret[i++] = p.second;
  return ret;
}

vector<FunctionDecl *> Context::get_functions(const str &name) { return _function_table.get(name); }

ASTBase *Context::owner() const { return _owner; }

vector<FunctionDecl *> Context::get_functions() { return _function_table.get_all(); }
