#include "ast/context.h"
#include "ast/function_table.h"

using namespace tanlang;

Context::Context(ASTBase *owner) : _owner(owner) {}

Decl *Context::get_decl(const str &name) {
  TAN_ASSERT(name != "");
  auto q = _decls.find(name);
  if (q != _decls.end()) {
    return q->second;
  }
  return nullptr;
}

void Context::set_decl(const str &name, Decl *decl) {
  TAN_ASSERT(!name.empty());
  _decls[name] = decl;
}

void Context::add_function_decl(FunctionDecl *func) { _function_table.set(func); }

vector<Decl *> Context::get_decls() {
  vector<Decl *> ret(_decls.size(), nullptr);
  size_t i = 0;
  for (const auto &p : _decls)
    ret[i++] = p.second;
  return ret;
}

vector<FunctionDecl *> Context::get_functions(const str &name) { return _function_table.get(name); }

ASTBase *Context::owner() const { return _owner; }

vector<FunctionDecl *> Context::get_functions() { return _function_table.get_all(); }

bool Context::merge(const Context &other) {
  for (auto [name, decl] : other._decls) {
    if (_decls.contains(name))
      return false;

    _decls[name] = decl;
  }

  return _function_table.merge(other._function_table);
}
