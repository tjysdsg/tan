#include "compiler_session.h"
#include "src/ast/ast_base.h"
#include "compiler.h"

using namespace tanlang;

CompilerSession::CompilerSession(const str &module_name, TargetMachine *tm)
    : _filename(module_name), _target_machine(tm) {
  _context = new LLVMContext();
  _builder = new IRBuilder<>(*_context);
  _module = new Module(module_name, *_context);
  _module->setDataLayout(_target_machine->createDataLayout());
  _module->setTargetTriple(_target_machine->getTargetTriple().str());
  auto opt_level = (llvm::CodeGenOpt::Level) Compiler::compile_config.opt_level;
  _target_machine->setOptLevel(opt_level);

  /// add the current debug info version into the module
  _module->addModuleFlag(Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
  _module->addModuleFlag(Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

  /// debug related
  _di_builder = new DIBuilder(*_module);
  _di_file = _di_builder->createFile(module_name, ".");
  _di_cu = _di_builder->createCompileUnit(llvm::dwarf::DW_LANG_C, _di_file, "tan compiler", false, "", 0);
  _di_scope = {_di_file};
}

CompilerSession::~CompilerSession() {
  delete _di_builder;
  delete _module;
  delete _builder;
  delete _context;
}

SourceManager *CompilerSession::get_source_manager() const { return _sm; }

void CompilerSession::set_source_manager(SourceManager *sm) { _sm = sm; }

LLVMContext *CompilerSession::get_context() { return _context; }

Module *CompilerSession::get_module() { return _module; }

void CompilerSession::emit_object(const str &filename) {
  _di_builder->finalize(); /// important: do this before any pass

  auto opt_level = _target_machine->getOptLevel();
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
  auto *tlii = new llvm::TargetLibraryInfoImpl(Triple(_module->getTargetTriple()));
  pm_builder->LibraryInfo = tlii;
  pm_builder->Inliner = llvm::createFunctionInliningPass();

  /// module pass
  PassManager mpm;
  mpm.add(createTargetTransformInfoWrapperPass(_target_machine->getTargetIRAnalysis()));
  pm_builder->populateModulePassManager(mpm);
  mpm.run(*_module);

  /// function pass
  FunctionPassManager fpm(_module);
  fpm.add(createTargetTransformInfoWrapperPass(_target_machine->getTargetIRAnalysis()));
  fpm.add(llvm::createVerifierPass());
  pm_builder->populateFunctionPassManager(fpm);
  fpm.doInitialization();
  for (auto &f: *_module) {
    if (f.getName() == "tan_main") { /// mark tan_main as used, prevent LLVM from deleting it
      llvm::appendToUsed(*_module, {(GlobalValue *) &f});
    }
    fpm.run(f);
  }
  fpm.doFinalization();

  /// generate object files
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
  if (ec) { report_error("Could not open file: " + ec.message()); }
  PassManager emit_pass;
  auto file_type = llvm::CGFT_ObjectFile;
  if (_target_machine->addPassesToEmitFile(emit_pass, dest, nullptr, file_type)) {
    report_error("Target machine can't emit a file of this type");
  }
  emit_pass.run(*_module);
  dest.flush();
}

DIScope *CompilerSession::get_current_di_scope() const { return _di_scope.back(); }

void CompilerSession::push_di_scope(DIScope *scope) { _di_scope.push_back(scope); }

void CompilerSession::pop_di_scope() { _di_scope.pop_back(); }

void CompilerSession::set_current_debug_location(size_t l, size_t c) {
  _builder->SetCurrentDebugLocation(DebugLoc::get((unsigned) (l + 1),
      (unsigned) (c + 1),
      this->get_current_di_scope()));
}

unsigned CompilerSession::get_ptr_size() const { return _target_machine->getPointerSizeInBits(0); }

DIFile *CompilerSession::get_di_file() const { return _di_file; }

DICompileUnit *CompilerSession::get_di_cu() const { return _di_cu; }

const str &CompilerSession::get_filename() const { return _filename; }
