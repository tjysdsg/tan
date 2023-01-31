#include "compiler/ast_context.h"
#include "ast/function_table.h"
#include "ast/source_manager.h"
#include "ast/ast_base.h"
#include "analysis/scope.h"

using namespace tanlang;

ASTContext *ASTContext::get_ctx_of_file(const str &filename) {
  auto *ret = file_to_ctx[filename];
  TAN_ASSERT(ret);
  return ret;
}

ASTContext::ASTContext(SourceManager *sm) : _sm(sm) {
  initialize_scope();
  file_to_ctx[sm->get_filename()] = this;
}

ASTContext::~ASTContext() {}

SourceManager *ASTContext::get_source_manager() const { return _sm; }

str ASTContext::get_source_location_str(SourceTraceable *p) const {
  return get_filename() + ":" + std::to_string(_sm->get_line(p->loc()));
}

Scope *ASTContext::get_current_scope() { return _scope.back(); }

Scope *ASTContext::push_scope() {
  auto r = new Scope;
  _scope.push_back(r);
  return r;
}

Scope *ASTContext::pop_scope() {
  if (_scope.size() == 1) {
    Error err("Cannot pop the outer-est scope");
    err.raise();
  }
  auto r = _scope.back();
  _scope.pop_back();
  return r;
}

void ASTContext::add_scoped_decl(const str &name, Decl *value) {
  TAN_ASSERT(!name.empty());
  get_current_scope()->_declared[name] = value;
}

Decl *ASTContext::get_scoped_decl(const str &name) {
  TAN_ASSERT(name != "");
  // search from the outer-est scope to the inner-est scope
  bool found = false;
  Decl *result = nullptr;
  auto scope = _scope.end(); // scope is an iterator
  --scope;
  while (!found && scope >= _scope.begin()) {
    auto search = (*scope)->_declared.find(name);
    if (search != (*scope)->_declared.end()) {
      found = true;
      result = search->second;
    }
    --scope;
  }
  return result;
}

TypeDecl *ASTContext::get_type_decl(const str &name) {
  TAN_ASSERT(name != "");
  { // search public
    auto q = _public_type_decls.find(name);
    if (q != _public_type_decls.end()) {
      return q->second;
    }
  }

  { // search private
    auto q = _private_type_decls.find(name);
    if (q != _private_type_decls.end()) {
      return q->second;
    }
  }

  return nullptr;
}

void ASTContext::add_type_decl(const str &name, TypeDecl *decl, bool is_public) {
  TAN_ASSERT(!name.empty());

  if (is_public)
    _public_type_decls[name] = decl;
  else
    _private_type_decls[name] = decl;
}

void ASTContext::add_function_decl(FunctionDecl *func, bool is_public) {
  if (is_public)
    _public_function_table.set(func);
  else
    _private_function_table.set(func);
}

vector<FunctionDecl *> ASTContext::get_public_functions() { return _public_function_table.get_all(); }

vector<TypeDecl *> ASTContext::get_public_type_decls() {
  vector<TypeDecl *> ret(_public_type_decls.size(), nullptr);
  size_t i = 0;
  for (const auto &p : _public_type_decls)
    ret[i++] = p.second;
  return ret;
}

vector<FunctionDecl *> ASTContext::get_functions(const str &name, bool include_imported) {
  auto ret = _public_function_table.get(name);
  auto privates = _private_function_table.get(name);
  if (include_imported) {
    ret.insert(ret.end(), privates.begin(), privates.end());
  }
  return ret;
}

str ASTContext::get_filename() const { return _sm->get_filename(); }

void ASTContext::initialize_scope() {
  _scope = vector<Scope *>();
  _scope.push_back(new Scope); // outer-est scope
}

Loop *ASTContext::get_current_loop() const {
  // search from the outer-est scope to the inner-est scope
  auto scope = _scope.end(); // scope is an iterator
  --scope;
  while (scope >= _scope.begin()) {
    Loop *loop = (*scope)->_current_loop;
    if (loop) {
      return loop;
    }
    --scope;
  }
  return nullptr;
}

void ASTContext::set_current_loop(Loop *loop) { get_current_scope()->_current_loop = loop; }
