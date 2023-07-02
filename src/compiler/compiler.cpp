#include "compiler/compiler.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "analysis/type_check.h"
#include "analysis/register_declarations.h"
#include "analysis/type_precheck.h"
#include "codegen/code_generator.h"
#include "common/compilation_unit.h"
#include "ast/intrinsic.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/context.h"
#include "lexer/source_file.h"
#include "parser/parser.h"
#include "llvm_api/llvm_include.h"
#include <filesystem>

using namespace tanlang;
namespace fs = std::filesystem;

Compiler::~Compiler() {}

Compiler::Compiler(const vector<str> &files) : _files(files) {
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

void Compiler::emit_object(CompilationUnit *cu, const str &out_file) {
  auto *cg = _cg[cu];
  cg->emit_to_file(out_file);
}

Value *Compiler::codegen(CompilationUnit *cu, bool print_ir) {
  TAN_ASSERT(_cg.find(cu) == _cg.end());
  auto *cg = _cg[cu] = new CodeGenerator(target_machine);
  auto *ret = cg->run(cu);

  if (print_ir) {
    cg->dump_ir();
  }

  return ret;
}

void Compiler::dump_ast() const {
  // TODO: dump_ast()
}

void Compiler::parse() {
  for (const str &file : _files) {
    SourceFile *source = new SourceFile();
    source->open(file);
    _srcs.push_back(source);

    // tokenization
    auto tokens = tokenize(source);

    auto *sm = new SourceManager(file, tokens);
    auto *parser = new Parser(sm);
    auto *ast = parser->parse();

    // register top-level declarations
    auto intrinsic_funcs = Intrinsic::GetIntrinsicFunctionDeclarations();
    for (auto *f : intrinsic_funcs) {
      ast->ctx()->set_function_decl(f);
    }

    _cu.push_back(new CompilationUnit(ast, sm));
  }
}

void Compiler::analyze() {
  for (auto *cu : _cu) {
    RegisterDeclarations rtld;
    rtld.run(cu);

    TypePrecheck tp;
    tp.run(cu);

    TypeCheck analyzer;
    analyzer.run(cu);
  }
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

const vector<CompilationUnit *> &Compiler::get_compilation_units() const { return _cu; }
