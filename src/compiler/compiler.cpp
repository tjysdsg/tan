#include "compiler.h"
#include "lexer.h"
#include "compiler_session.h"
#include "intrinsic.h"
#include "reader.h"
#include "parser.h"

using namespace tanlang;

Compiler::~Compiler() {
  Compiler::sessions.erase(_filename);
  delete _compiler_session;
}

Compiler::Compiler(std::string filename) : _filename(filename) {
  /// target machine and data layout
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
  if (!Compiler::target_machine) {
    Compiler::target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }
  _compiler_session = new CompilerSession(filename, Compiler::target_machine);
}

void Compiler::emit_object(const std::string &filename) { _compiler_session->emit_object(filename); }

Value *Compiler::codegen() {
  TAN_ASSERT(_ast);
  TAN_ASSERT(_compiler_session);
  TAN_ASSERT(_compiler_session->get_module());
  Intrinsic::InitCodegen(_compiler_session);
  auto *ret = _ast->codegen(_compiler_session);
  return ret;
}

void Compiler::dump_ir() const {
  TAN_ASSERT(_compiler_session);
  TAN_ASSERT(_compiler_session->get_module());
  _compiler_session->get_module()->print(llvm::outs(), nullptr);
}

void Compiler::dump_ast() const {
  TAN_ASSERT(_ast);
  _ast->printTree();
}

void Compiler::parse() {
  Reader reader;
  reader.open(_filename);
  auto tokens = tokenize(&reader);
  auto *parser = new Parser(tokens, std::string(_filename), _compiler_session);
  _ast = parser->parse();
}

void Compiler::ParseFile(std::string filename) {
  auto compiler = std::make_shared<Compiler>(filename);
  compiler->parse();
  Compiler::sub_compilers.push_back(compiler);
}

TargetMachine *Compiler::GetDefaultTargetMachine() {
  TAN_ASSERT(Compiler::target_machine);
  return Compiler::target_machine;
}
