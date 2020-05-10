#include "compiler_session.h"
#include "src/scope.h"
#include "src/compiler/function_table.h"
#include "compiler.h"
#include "base.h"
#include "src/ast/ast_node.h"

using namespace tanlang;

void CompilerSession::initialize_scope() {
  _scope = std::vector<std::shared_ptr<Scope>>();
  _scope.push_back(std::make_shared<Scope>()); // outer-est scope
  _scope.back()->_code_block = _builder->GetInsertBlock();
}

CompilerSession::CompilerSession(const std::string &module_name, TargetMachine *target_machine) : _target_machine(
    target_machine) {
  _context = new LLVMContext();
  _builder = new IRBuilder<>(*_context);
  _module = new Module(module_name, *_context);
  init_llvm();
  // add the current debug info version into the module
  _module->addModuleFlag(Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
  _module->addModuleFlag(Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
  _di_builder = std::make_unique<DIBuilder>(*_module);
  _di_file = _di_builder->createFile(module_name, ".");
  _di_cu = _di_builder->createCompileUnit(llvm::dwarf::DW_LANG_C, _di_file, "tan compiler", false, "", 0);
  _di_scope = {_di_file};
  _function_table = std::make_shared<FunctionTable>();
  initialize_scope();
}

CompilerSession::~CompilerSession() {}

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
  TAN_ASSERT(name != "");
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

LLVMContext *CompilerSession::get_context() { return _context; }

IRBuilder<> *CompilerSession::get_builder() { return _builder; }

Module *CompilerSession::get_module() { return _module; }

void CompilerSession::set_code_block(BasicBlock *block) {
  _scope.back()->_code_block = block;
}

BasicBlock *CompilerSession::get_code_block() const {
  return _scope.back()->_code_block;
}

void CompilerSession::init_llvm() {
  _module->setDataLayout(_target_machine->createDataLayout());
  _module->setTargetTriple(_target_machine->getTargetTriple().str());
  auto opt_level = (llvm::CodeGenOpt::Level) Compiler::compile_config.opt_level;
  _target_machine->setOptLevel(opt_level);
  bool debug = opt_level == llvm::CodeGenOpt::Level::None;

  /// pass manager builder
  auto *pm_builder = new PassManagerBuilder();
  pm_builder->OptLevel = opt_level;
  pm_builder->SizeLevel = 0; // TODO: optimize for size?
  pm_builder->DisableTailCalls = debug;
  pm_builder->DisableUnrollLoops = debug;
  pm_builder->SLPVectorize = !debug;
  pm_builder->LoopVectorize = !debug;
  pm_builder->RerollLoops = !debug;
  pm_builder->NewGVN = !debug;
  pm_builder->DisableGVNLoadPRE = !debug;
  pm_builder->MergeFunctions = !debug;
  pm_builder->VerifyInput = true;
  pm_builder->VerifyOutput = true;
  pm_builder->PrepareForLTO = false;
  pm_builder->PrepareForThinLTO = false;
  pm_builder->PerformThinLTO = false;
  llvm::TargetLibraryInfoImpl tlii(Triple(_module->getTargetTriple()));
  pm_builder->LibraryInfo = &tlii;
  if (debug) { pm_builder->Inliner = llvm::createAlwaysInlinerLegacyPass(false); }

  /// function pass
  _fpm = std::make_unique<FunctionPassManager>(_module);
  auto *tliwp = new llvm::TargetLibraryInfoWrapperPass(tlii);
  _fpm->add(tliwp);
  _fpm->add(createTargetTransformInfoWrapperPass(_target_machine->getTargetIRAnalysis()));
  _fpm->add(llvm::createVerifierPass());
  pm_builder->populateFunctionPassManager(*_fpm.get());
  _fpm->doInitialization();

  /// module pass
  _mpm = std::make_unique<PassManager>();
  _mpm->add(createTargetTransformInfoWrapperPass(_target_machine->getTargetIRAnalysis()));
  pm_builder->populateModulePassManager(*_mpm.get());
}

void CompilerSession::emit_object(const std::string &filename) {
  _di_builder->finalize(); /// important: do this before any pass

  /// run function pass on all functions in the current module
  for (auto &f: *_module) {
    if (f.getName() == "main") { /// special case for main function
      // mark as used, prevent LLVM from deleting it
      llvm::appendToUsed(*_module, {(GlobalValue *) &f});
    }
    _fpm->run(f);
  }
  _fpm->doFinalization();

  /// generate object files
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
  if (ec) { throw std::runtime_error("Could not open file: " + ec.message()); }

  auto file_type = llvm::CGFT_ObjectFile;
  if (_target_machine->addPassesToEmitFile(*_mpm.get(), dest, nullptr, file_type)) {
    throw std::runtime_error("Target machine can't emit a file of this type");
  }
  _mpm->run(*_module);
  dest.flush();
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

void CompilerSession::AddPublicFunction(const std::string &filename, ASTNodePtr func) {
  auto f = ast_cast<ASTFunction>(func);
  TAN_ASSERT(f);
  auto &pf = CompilerSession::public_func;
  if (pf.find(filename) == pf.end()) {
    pf[filename] = std::make_shared<FunctionTable>();
  }
  pf[filename]->set(f);
}

std::vector<ASTFunctionPtr> CompilerSession::GetPublicFunctions(const std::string &filename) {
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
  TAN_ASSERT(f);
  _function_table->set(f);
}

std::vector<ASTFunctionPtr> CompilerSession::get_functions(const std::string &name) { return _function_table->get(name); }

DIFile *CompilerSession::get_di_file() const { return _di_file; }

DICompileUnit *CompilerSession::get_di_cu() const { return _di_cu; }

std::shared_ptr<ASTLoop> CompilerSession::get_current_loop() const { return _current_loop; }

void CompilerSession::set_current_loop(std::shared_ptr<ASTLoop> loop) { _current_loop = loop; }
