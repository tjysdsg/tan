#include "compiler.h"

namespace tanlang {

Compiler::Compiler(const std::shared_ptr<Module> &module) : _llvm_module(module) {
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
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  _target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  module->setDataLayout(_target_machine->createDataLayout());
  module->setTargetTriple(target_triple);
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
