#include "compiler.h"
#include "lexer.h"
#include "token.h"
#include "compiler_session.h"
#include "src/analysis/analyzer.h"
#include "src/codegen/code_generator.h"
#include "src/ast/intrinsic.h"
#include "src/ast/ast_context.h"
#include "reader.h"
#include "parser.h"

using namespace tanlang;

Compiler::~Compiler() {
  Compiler::sessions.erase(_filename);
  delete _cs;
  delete _ctx;
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
  if (!target) {
    Error err(error);
    err.raise();
  }
  if (!Compiler::target_machine) {
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    /// relocation model
    auto RM = llvm::Reloc::Model::PIC_;
    Compiler::target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }
  _ctx = new ASTContext(filename);
  _cs = new CompilerSession(filename, Compiler::target_machine);
}

void Compiler::emit_object(const str &filename) { _cs->emit_object(filename); }

Value *Compiler::codegen() {
  TAN_ASSERT(_ast);
  TAN_ASSERT(_ctx);
  TAN_ASSERT(_cs);
  TAN_ASSERT(_cs->get_module());
  Intrinsic::InitCodegen(_cs);
  CodeGenerator cg(_cs, _ctx);
  auto *ret = cg.codegen(_ast);
  return ret;
}

void Compiler::dump_ir() const {
  TAN_ASSERT(_cs);
  TAN_ASSERT(_cs->get_module());
  _cs->get_module()->print(llvm::outs(), nullptr);
}

void Compiler::dump_ast() const {
  TAN_ASSERT(_ast);
  _ast->printTree();
}

void Compiler::parse() {
  Reader reader;
  reader.open(_filename);

  auto tokens = tokenize(&reader);
  auto *sm = new SourceManager(_filename, tokens);
  _ctx->set_source_manager(sm);
  _cs->set_source_manager(sm);

  auto *parser = new Parser(_ctx);
  _ast = parser->parse();

  Intrinsic::InitAnalysis(_ctx);
  Analyzer analyzer(_ctx);
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
  for (const auto &rel: Compiler::import_dirs) {
    auto p = fs::path(rel) / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) { ret.push_back(p.string()); }
  }
  return ret;
}
