#include "compiler_session.h"
#include "src/scope.h"
#include "src/ast/ast_base.h"
#include "src/compiler/function_table.h"
#include "compiler.h"
#include "base.h"

using namespace tanlang;

void CompilerSession::initialize_scope() {
  _scope = vector<Scope *>();
  _scope.push_back(new Scope); // outer-est scope
}

CompilerSession::CompilerSession(const str &module_name, TargetMachine *tm)
    : _filename(module_name), _target_machine(tm) {
  _context = new LLVMContext();
  _builder = new IRBuilder<>(*_context);
  _module = new Module(module_name, *_context);
  init_llvm(); // FIXME: call this during codegen instead of now

  /// add the current debug info version into the module
  _module->addModuleFlag(Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
  _module->addModuleFlag(Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

  /// debug related
  _di_builder = new DIBuilder(*_module);
  _di_file = _di_builder->createFile(module_name, ".");
  _di_cu = _di_builder->createCompileUnit(llvm::dwarf::DW_LANG_C, _di_file, "tan compiler", false, "", 0);
  _di_scope = {_di_file};

  _function_table = new FunctionTable;

  /// scopes
  initialize_scope();
}

CompilerSession::~CompilerSession() {}

Scope *CompilerSession::get_current_scope() { return _scope.back(); }

Scope *CompilerSession::push_scope() {
  auto r = new Scope;
  _scope.push_back(r);
  return r;
}

void CompilerSession::push_scope(Scope *scope) { _scope.push_back(scope); }

Scope *CompilerSession::pop_scope() {
  if (_scope.size() == 1) { report_error("Cannot pop the outer-est scope"); }
  auto r = _scope.back();
  _scope.pop_back();
  return r;
}

void CompilerSession::add(const str &name, ASTBase *value) {
  get_current_scope()->_named[name] = value;
}

void CompilerSession::set(const str &name, ASTBase *value) {
  auto scope = _scope.end();
  bool found = false;
  --scope;
  while (scope >= _scope.begin()) {
    auto search = (*scope)->_named.find(name);
    if (search != (*scope)->_named.end()) {
      found = true;
      break;
    }
    --scope;
  }
  if (found) { (*scope)->_named[name] = value; }
  else { report_error("Cannot set the value of " + name); }
}

ASTBase *CompilerSession::get(const str &name) {
  TAN_ASSERT(name != "");
  // search from the outer-est scope to the inner-est scope
  bool found = false;
  ASTBase *result = nullptr;
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

void CompilerSession::add_type_decl(const str &name, Decl *ty) {
  _type_decls[name] = ty;
}

// TODO: return std::optional
Decl *CompilerSession::get_type_decl(const str &name) {
  TAN_ASSERT(name != "");
  auto q = _type_decls.find(name);
  if (q != _type_decls.end()) {
    return q->second;
  }
  return nullptr;
}

LLVMContext *CompilerSession::get_context() { return _context; }

Module *CompilerSession::get_module() { return _module; }

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

void CompilerSession::emit_object(const str &filename) {
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
  if (ec) { report_error("Could not open file: " + ec.message()); }

  auto file_type = llvm::CGFT_ObjectFile;
  if (_target_machine->addPassesToEmitFile(*_mpm.get(), dest, nullptr, file_type)) {
    report_error("Target machine can't emit a file of this type");
  }
  _mpm->run(*_module);
  dest.flush();
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

void CompilerSession::AddPublicFunction(const str &filename, FunctionDecl *func) {
  auto &pf = CompilerSession::public_func;
  if (pf.find(filename) == pf.end()) {
    pf[filename] = new FunctionTable;
  }
  pf[filename]->set(func);
}

vector<FunctionDecl *> CompilerSession::GetPublicFunctions(const str &filename) {
  auto &pf = CompilerSession::public_func;
  auto funcs = pf.find(filename);
  if (funcs != pf.end()) {
    auto fuck = funcs->second;
    return fuck->get_all();
  }
  return {};
}

unsigned CompilerSession::get_ptr_size() const { return _target_machine->getPointerSizeInBits(0); }

void CompilerSession::add_function(FunctionDecl *func) { _function_table->set(func); }

vector<FunctionDecl *> CompilerSession::get_functions(const str &name) { return _function_table->get(name); }

DIFile *CompilerSession::get_di_file() const { return _di_file; }

DICompileUnit *CompilerSession::get_di_cu() const { return _di_cu; }

Loop *CompilerSession::get_current_loop() const { return _current_loop; }

void CompilerSession::set_current_loop(Loop *loop) { _current_loop = loop; }

SourceManager *CompilerSession::get_source_manager() const { return _source_manager; }

void CompilerSession::set_source_manager(SourceManager *source_manager) { _source_manager = source_manager; }

str CompilerSession::get_source_location_str(SourceTraceable *p) const {
  return _filename + ":" + std::to_string(_source_manager->get_line(p->get_loc()));
}
