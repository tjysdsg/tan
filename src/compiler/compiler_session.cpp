#include "compiler_session.h"

namespace tanlang {

CompilerSession::CompilerSession() {
  _context = std::make_unique<LLVMContext>();
  _builder = std::make_unique<IRBuilder<>>(*_context);
  _module = std::make_unique<Module>("main", *_context);
  _scope = std::vector<std::shared_ptr<Scope >>();
  _scope.push_back(std::make_shared<Scope>()); // outer-est scope
}

CompilerSession::CompilerSession(const std::string &module_name) {
  _context = std::make_unique<LLVMContext>();
  _builder = std::make_unique<IRBuilder<>>(*_context);
  _module = std::make_unique<Module>(module_name, *_context);
  _scope = std::vector<std::shared_ptr<Scope >>();
  _scope.push_back(std::make_shared<Scope>()); // outer-est scope
}

std::shared_ptr<Scope> CompilerSession::get_current_scope() {
  return _scope.back();
}

std::shared_ptr<Scope> CompilerSession::push_scope() {
  auto r = std::make_shared<Scope>();
  _scope.push_back(r);
  return r;
}

std::shared_ptr<Scope> CompilerSession::pop_scope() {
  if (_scope.size() == 1) {
    throw std::runtime_error("Cannot pop the outer-est scope");
  }
  auto r = _scope.back();
  _scope.pop_back();
  return r;
}

void CompilerSession::add(const std::string &name, Value *value) {
  get_current_scope()->_named.insert(std::make_pair(name, value));
}

void CompilerSession::set(const std::string &name, Value *value) {
  get_current_scope()->_named[name] = value;
}

Value *CompilerSession::get(const std::string &name) {
  // search from the outer-est scope to the inner-est scope
  bool found = false;
  Value *result = nullptr;
  auto scope = _scope.end(); // scope is an iterator
  --scope;
  // std::shared_ptr<Scope> scope = get_current_scope();
  while (!found && scope >= _scope.begin()) {
    auto search = (*scope)->_named.find(name);
    if (search != (*scope)->_named.end()) {
      found = true;
      result = search->second;
    }
    --scope;
  }
  return result;
}

std::unique_ptr<LLVMContext> &CompilerSession::get_context() {
  return _context;
}

std::unique_ptr<IRBuilder<>> &CompilerSession::get_builder() {
  return _builder;
}

std::unique_ptr<Module> &CompilerSession::get_module() {
  return _module;
}
} // namespace tanlang
