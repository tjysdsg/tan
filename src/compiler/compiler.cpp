#include "compiler.h"
#include "lexer.h"
#include "token.h"
#include "compiler_session.h"
#include "src/analysis/analyzer.h"
#include "src/codegen/code_generator.h"
#include "src/ast/intrinsic.h"
#include "reader.h"
#include "parser.h"

using namespace tanlang;

Compiler::~Compiler() {
  Compiler::sessions.erase(_filename);
  delete _compiler_session;
}

Compiler::Compiler(const str &filename) : _filename(filename) {
  /// target machine and data layout
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();
  auto target_triple = llvm::sys::getDefaultTargetTriple();
  str error;
  auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
  if (!target) { report_error(error); }
  if (!Compiler::target_machine) {
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    /// relocation model
    auto RM = llvm::Reloc::Model::PIC_;
    Compiler::target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }
  _compiler_session = new CompilerSession(filename, Compiler::target_machine);
}

void Compiler::emit_object(const str &filename) { _compiler_session->emit_object(filename); }

Value *Compiler::codegen() {
  TAN_ASSERT(_ast);
  TAN_ASSERT(_compiler_session);
  TAN_ASSERT(_compiler_session->get_module());
  Intrinsic::InitCodegen(_compiler_session);
  CodeGenerator cg(_compiler_session);
  auto *ret = cg.codegen(_ast);
  return ret;
}

void Compiler::dump_ir() const {
  TAN_ASSERT(_compiler_session);
  TAN_ASSERT(_compiler_session->get_module());
  _compiler_session->get_module()->print(llvm::outs(), nullptr);
}

void Compiler::dump_ast() const {
  TAN_ASSERT(_ast);
  // TODO: fix print tree
  // _ast->printTree();
}

void Compiler::parse() {
  Reader reader;
  reader.open(_filename);
  auto tokens = tokenize(&reader);
  auto *parser = new Parser(tokens, str(_filename), _compiler_session);
  _ast = parser->parse();

  // TODO: separate parsing and analyzing phase
  Intrinsic::InitAnalysis(_compiler_session);
  Analyzer analyzer(_compiler_session);
  analyzer.analyze(_ast);
}

void Compiler::ParseFile(const str &filename) {
  auto compiler = new Compiler(filename);
  compiler->parse();
  Compiler::sub_compilers.push_back(compiler);
}

TargetMachine *Compiler::GetDefaultTargetMachine() {
  TAN_ASSERT(Compiler::target_machine);
  return Compiler::target_machine;
}

vector<str> Compiler::resolve_import(const str &callee_path, const str &import_name) {
  vector<str> ret{};
  auto import_path = fs::path(import_name);
  /// search relative to callee's path
  {
    auto p = fs::path(callee_path).parent_path() / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) { ret.push_back(p.string()); }
  }
  /// search relative to directories in Compiler::import_dirs
  for (const auto &rel : Compiler::import_dirs) {
    auto p = fs::path(rel) / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) { ret.push_back(p.string()); }
  }
  return ret;
}
