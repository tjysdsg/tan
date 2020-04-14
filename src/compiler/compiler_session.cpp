#include "compiler_session.h"
#include "stack_trace.h"

namespace tanlang {

void CompilerSession::initialize_scope() {
  _scope = std::vector<std::shared_ptr<Scope>>();
  _scope.push_back(std::make_shared<Scope>()); // outer-est scope
  _scope.back()->_code_block = _builder->GetInsertBlock();
}

CompilerSession::CompilerSession(const std::string &module_name) {
  _context = std::make_unique<LLVMContext>();
  _builder = std::make_unique<IRBuilder<>>(*_context);
  _module = std::make_unique<Module>(module_name, *_context);
  initialize_scope();
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

void CompilerSession::add(const std::string &name, std::shared_ptr<ASTNode> value) {
  get_current_scope()->_named.insert(std::make_pair(name, value));
}

void CompilerSession::set(const std::string &name, std::shared_ptr<ASTNode> value) {
  // search from the outer-est scope to the inner-est scope
  auto scope = _scope.end(); // scope is an iterator
  --scope;
  while (scope >= _scope.begin()) {
    auto search = (*scope)->_named.find(name);
    if (search != (*scope)->_named.end()) {
      throw std::runtime_error("Cannot set the value of " + name);
    }
    --scope;
  }
  (*scope)->_named[name] = value;
}

std::shared_ptr<ASTNode> CompilerSession::get(const std::string &name) {
  // search from the outer-est scope to the inner-est scope
  bool found = false;
  std::shared_ptr<ASTNode> result = nullptr;
  auto scope = _scope.end(); // scope is an iterator
  --scope;
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

LLVMContext *CompilerSession::get_context() {
  return _context.get();
}

std::unique_ptr<IRBuilder<>> &CompilerSession::get_builder() {
  return _builder;
}

std::unique_ptr<Module> &CompilerSession::get_module() {
  return _module;
}

void CompilerSession::set_code_block(BasicBlock *block) {
  _scope.back()->_code_block = block;
}

BasicBlock *CompilerSession::get_code_block() const {
  return _scope.back()->_code_block;
}

} // namespace tanlang
