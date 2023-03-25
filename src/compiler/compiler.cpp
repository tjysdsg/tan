#include "compiler/compiler.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "analysis/analyzer.h"
#include "analysis/register_top_level_declarations.h"
#include "codegen/code_generator.h"
#include "ast/intrinsic.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/context.h"
#include "lexer/reader.h"
#include "parser/parser.h"
#include "llvm_api/llvm_include.h"
#include <filesystem>

using namespace tanlang;
namespace fs = std::filesystem;

Compiler::~Compiler() {
  if (_ast)
    delete _ast;
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
}

void Compiler::emit_object(const str &filename) { _cg->emit_to_file(filename); }

Value *Compiler::codegen() {
  TAN_ASSERT(_ast);
  TAN_ASSERT(!_cg);
  _cg = new CodeGenerator(_sm, target_machine);
  auto *ret = _cg->codegen(_ast);
  return ret;
}

void Compiler::dump_ir() const {
  TAN_ASSERT(_cg);
  _cg->dump_ir();
}

void Compiler::dump_ast() const {
  TAN_ASSERT(_ast);
  _ast->printTree();
}

void Compiler::parse() {
  Reader reader;
  reader.open(_filename);

  // tokenization
  auto tokens = tokenize(&reader);

  // syntactic parsing
  _sm = new SourceManager(_filename, tokens);
  auto *parser = new Parser(_sm);
  _ast = parser->parse();

  // register intrinsic functions
  auto intrinsic_funcs = Intrinsic::GetIntrinsicFunctionDeclarations();
  for (auto *f : intrinsic_funcs) {
    _ast->ctx()->add_function_decl(f);
  }

  RegisterTopLevelDeclarations rtld(_sm);
  rtld.run(_ast);

  // symbol dependency analysis
  _analyzer = new Analyzer(_sm);
  _analyzer->stage1(_ast);
}

void Compiler::analyze() {
  vector<ASTBase *> sorted = _analyzer->sorted_unresolved_symbols();

  // std::cout << "Sorted unresolved symbol dependency:\n";
  // for (auto *d : sorted) {
  //   str name = ast_cast<Decl>(d)->get_name();
  //   std::cout << name << '\n';
  // }

  Analyzer analyzer(_sm);
  analyzer.stage2(_ast, sorted);
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
    if (fs::exists(p)) {
      ret.push_back(p.string());
    }
  }
  /// search relative to directories in Compiler::import_dirs
  for (const auto &rel : Compiler::import_dirs) {
    auto p = fs::path(rel) / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) {
      ret.push_back(p.string());
    }
  }
  return ret;
}

Program *Compiler::get_root_ast() const { return _ast; }
