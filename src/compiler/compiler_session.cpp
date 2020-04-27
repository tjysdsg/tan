#include "compiler_session.h"
#include "stack_trace.h"

namespace tanlang {

std::unordered_map<std::string, FunctionTablePtr> CompilerSession::public_func{};

void CompilerSession::initialize_scope() {
  _scope = std::vector<std::shared_ptr<Scope>>();
  _scope.push_back(std::make_shared<Scope>()); // outer-est scope
  _scope.back()->_code_block = _builder->GetInsertBlock();
}

CompilerSession::CompilerSession(const std::string &module_name, TargetMachine *target_machine) : _target_machine(
    target_machine) {
  _context = std::make_unique<LLVMContext>();
  _builder = std::make_unique<IRBuilder<>>(*_context);
  _module = std::make_unique<Module>(module_name, *_context);
  init_llvm();
  // add the current debug info version into the module
  _module->addModuleFlag(Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
  _module->addModuleFlag(Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
  _di_builder = std::make_unique<DIBuilder>(*_module.get());
  _di_file = _di_builder->createFile(module_name, ".");
  _di_cu = _di_builder->createCompileUnit(llvm::dwarf::DW_LANG_C, _di_file, "tan compiler", false, "", 0);
  _di_scope = {_di_file};
  _function_table = std::make_shared<FunctionTable>();
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

void CompilerSession::push_scope(std::shared_ptr<Scope> scope) { _scope.push_back(scope); }

std::shared_ptr<Scope> CompilerSession::pop_scope() {
  if (_scope.size() == 1) {
    throw std::runtime_error("Cannot pop the outer-est scope");
  }
  auto r = _scope.back();
  _scope.pop_back();
  return r;
}

void CompilerSession::add(const std::string &name, ASTNodePtr value) {
  get_current_scope()->_named.insert(std::make_pair(name, value));
}

void CompilerSession::set(const std::string &name, ASTNodePtr value) {
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

ASTNodePtr CompilerSession::get(const std::string &name) {
  assert(name != "");
  // search from the outer-est scope to the inner-est scope
  bool found = false;
  ASTNodePtr result = nullptr;
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

std::unique_ptr<FunctionPassManager> &CompilerSession::get_function_pass_manager() {
  return _fpm;
}

void CompilerSession::set_code_block(BasicBlock *block) {
  _scope.back()->_code_block = block;
}

BasicBlock *CompilerSession::get_code_block() const {
  return _scope.back()->_code_block;
}

void CompilerSession::init_llvm() {
  _module->setDataLayout(_target_machine->createDataLayout());
  _module->setTargetTriple(_target_machine->getTargetTriple().str());
  // init function pass
  _fpm = std::make_unique<FunctionPassManager>(_module.get());
  _fpm->add(llvm::createInstructionCombiningPass());
  _fpm->add(llvm::createReassociatePass());
  _fpm->add(llvm::createGVNPass());
  _fpm->add(llvm::createCFGSimplificationPass());
  _fpm->doInitialization();
}

void CompilerSession::finalize_codegen() {
  _di_builder->finalize();
}

std::unique_ptr<DIBuilder> &CompilerSession::get_di_builder() {
  return _di_builder;
}

DIScope *CompilerSession::get_current_di_scope() const {
  return _di_scope.back();
}

void CompilerSession::push_di_scope(DIScope *scope) {
  _di_scope.push_back(scope);
}

void CompilerSession::pop_di_scope() {
  _di_scope.pop_back();
}

void CompilerSession::set_current_debug_location(size_t l, size_t c) {
  _builder->SetCurrentDebugLocation(DebugLoc::get((unsigned) (l + 1),
      (unsigned) (c + 1),
      this->get_current_di_scope()));
}

void CompilerSession::add_public_function(const std::string &filename, ASTNodePtr func) {
  auto f = ast_cast<ASTFunction>(func);
  assert(f);
  auto &pf = CompilerSession::public_func;
  if (pf.find(filename) == pf.end()) {
    pf[filename] = std::make_shared<FunctionTable>();
  }
  pf[filename]->set(f);
}

std::vector<ASTFunctionPtr> CompilerSession::get_public_functions(const std::string &filename) {
  auto &pf = CompilerSession::public_func;
  auto funcs = pf.find(filename);
  if (funcs != pf.end()) {
    auto fuck = funcs->second;
    return fuck->get_all();
  }
  return {};
}

unsigned CompilerSession::get_ptr_size() const { return _target_machine->getPointerSizeInBits(0); }

void CompilerSession::add_function(ASTNodePtr func) {
  auto f = ast_cast<ASTFunction>(func);
  assert(f);
  _function_table->set(f);
}

std::vector<ASTFunctionPtr> CompilerSession::get_functions(const std::string &name) {
  std::vector<ASTFunctionPtr> ret{};
  return _function_table->get(name);
}

DIFile *CompilerSession::get_di_file() const { return _di_file; }

DICompileUnit *CompilerSession::get_di_cu() const { return _di_cu; }

} // namespace tanlang
