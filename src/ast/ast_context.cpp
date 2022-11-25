#include "ast/ast_context.h"
#include "ast/function_table.h"
#include "ast/source_manager.h"
#include "ast/ast_base.h"
#include "src/scope.h"

using namespace tanlang;

void ASTContext::AddPublicFunction(const str &filename, FunctionDecl *func) {
  auto &pf = ASTContext::public_func;
  if (pf.find(filename) == pf.end()) {
    pf[filename] = new FunctionTable;
  }
  pf[filename]->set(func);
}

vector<FunctionDecl *> ASTContext::GetPublicFunctions(const str &filename) {
  auto &pf = ASTContext::public_func;
  auto funcs = pf.find(filename);
  if (funcs != pf.end()) {
    auto fuck = funcs->second;
    return fuck->get_all();
  }
  return {};
}

ASTContext::ASTContext(str filename) : _filename(std::move(filename)) {
  _function_table = new FunctionTable;
  initialize_scope();
}

ASTContext::~ASTContext() { delete _function_table; }

SourceManager *ASTContext::get_source_manager() const { return _sm; }

void ASTContext::set_source_manager(SourceManager *sm) { _sm = sm; }

str ASTContext::get_source_location_str(SourceTraceable *p) const {
  return _filename + ":" + std::to_string(_sm->get_line(p->loc()));
}

Scope *ASTContext::get_current_scope() { return _scope.back(); }

Scope *ASTContext::push_scope() {
  auto r = new Scope;
  _scope.push_back(r);
  return r;
}

void ASTContext::push_scope(Scope *scope) { _scope.push_back(scope); }

Scope *ASTContext::pop_scope() {
  if (_scope.size() == 1) {
    Error err("Cannot pop the outer-est scope");
    err.raise();
  }
  auto r = _scope.back();
  _scope.pop_back();
  return r;
}

void ASTContext::add_decl(const str &name, Decl *value) { get_current_scope()->_declared[name] = value; }

Decl *ASTContext::get_decl(const str &name) {
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

Decl *ASTContext::get_type_decl(const str &name) {
  TAN_ASSERT(name != "");
  auto q = _type_decls.find(name);
  if (q != _type_decls.end()) {
    return q->second;
  }
  return nullptr;
}

void ASTContext::add_type_decl(const str &name, Decl *decl) { _type_decls[name] = decl; }

void ASTContext::add_function(FunctionDecl *func) { _function_table->set(func); }

vector<FunctionDecl *> ASTContext::get_functions(const str &name) { return _function_table->get(name); }

const str &ASTContext::get_filename() const { return _filename; }

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
    if (loop) { return loop; }
    --scope;
  }
  return nullptr;
}

void ASTContext::set_current_loop(Loop *loop) { get_current_scope()->_current_loop = loop; }
