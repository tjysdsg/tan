#include "compiler.h"
#include "libtanc.h"
#include "compiler_session.h"
#include "intrinsic.h"
#include "reader.h"
#include "parser.h"

namespace tanlang {

std::unordered_map<std::string, CompilerSession *> Compiler::sessions{};

Compiler::~Compiler() { delete _target_machine; }

Compiler::Compiler(std::string filename) : _filename(filename) {
  { /// target machine and data layout
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    if (!target) { throw std::runtime_error(error); }
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    /// relocation model
    auto RM = llvm::Reloc::Model::PIC_;
    _target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }
  _compiler_session = new CompilerSession(filename, _target_machine);
  Compiler::set_compiler_session(filename, _compiler_session);
}

void Compiler::emit_object(const std::string &filename) {
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);

  if (ec) {
    throw std::runtime_error("Could not open file: " + ec.message());
  }
  llvm::legacy::PassManager pass;
  auto file_type = llvm::CGFT_ObjectFile;

  if (_target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
    throw std::runtime_error("Target machine can't emit a file of this type");
  }
  pass.run(*_compiler_session->get_module());
  dest.flush();
}

Value *Compiler::codegen() {
  assert(_ast);
  assert(_compiler_session);
  assert(_compiler_session->get_module());
  Intrinsic::InitCodegen(_compiler_session);
  auto *ret = _ast->codegen(_compiler_session);
  _compiler_session->finalize_codegen();
  llvm::verifyModule(*_compiler_session->get_module());
  return ret;
}

void Compiler::dump_ir() const {
  assert(_compiler_session);
  assert(_compiler_session->get_module());
  _compiler_session->get_module()->print(llvm::outs(), nullptr);
}

void Compiler::dump_ast() const {
  assert(_ast);
  _ast->printTree();
}

CompilerSession *Compiler::get_compiler_session(const std::string &filename) {
  if (Compiler::sessions.find(filename) == Compiler::sessions.end()) { return nullptr; }
  return Compiler::sessions[filename];
}

void Compiler::set_compiler_session(const std::string &filename, CompilerSession *compiler_session) {
  Compiler::sessions[filename] = compiler_session;
}

void Compiler::parse() {
  Reader reader;
  reader.open(_filename);
  auto tokens = tanlang::tokenize(&reader);
  auto *parser = new Parser(tokens, std::string(_filename));
  _ast = parser->parse();
}

void Compiler::ParseFile(std::string filename) {
  Compiler compiler(filename);
  compiler.parse();
}

} // namespace tanlang
