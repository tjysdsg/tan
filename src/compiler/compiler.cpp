#include "compiler.h"
#include "libtanc.h"
#include "compiler_session.h"

namespace tanlang {

Compiler::~Compiler() {
  delete _llvm_module;
  delete _target_machine;
}

Compiler::Compiler(CompilerSession *compiler_session, TanCompilation *config) {
  _llvm_module = compiler_session->get_module().release();
  auto target_triple = llvm::sys::getDefaultTargetTriple();

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string error;
  auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);

  // Throw an error if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!target) {
    throw std::runtime_error(error);
  }

  auto CPU = "generic";
  auto features = "";

  llvm::TargetOptions opt;
  /// relocation model
  auto RM = llvm::Reloc::Model::PIC_;
  if (config->type == SLIB) {
    RM = llvm::Reloc::Model::Static;
  }
  _target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  _llvm_module->setDataLayout(_target_machine->createDataLayout());
  _llvm_module->setTargetTriple(target_triple);
  compiler_session->finalize_codegen();
}

void Compiler::emit_object(const std::string &filename) {
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    throw std::runtime_error("Could not open file: " + ec.message());
  }
  llvm::legacy::PassManager pass;
  auto file_type = llvm::LLVMTargetMachine::CGFT_ObjectFile;

  if (_target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
    throw std::runtime_error("Target machine can't emit a file of this type");
  }

  pass.run(*_llvm_module);
  dest.flush();
}

} // namespace tanlang
