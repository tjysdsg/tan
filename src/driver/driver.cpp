#include "driver/driver.h"
#include "lexer/lexer.h"
#include "source_file/token.h"
#include "analysis/type_check.h"
#include "analysis/register_declarations.h"
#include "analysis/type_precheck.h"
#include "codegen/code_generator.h"
#include "common/compilation_unit.h"
#include "ast/intrinsic.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/context.h"
#include "source_file/source_file.h"
#include "parser/parser.h"
#include "llvm_api/llvm_include.h"
#include <filesystem>

using namespace tanlang;
namespace fs = std::filesystem;

CompilerDriver::~CompilerDriver() {
  for (const auto &it : _cg) {
    delete it.second;
  }
  _cg.clear();

  for (CompilationUnit *c : _cu) {
    delete c;
  }
  _cu.clear();

  for (SourceFile *s : _srcs) {
    delete s;
  }
}

CompilerDriver::CompilerDriver(const vector<str> &files) : _files(files) {
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
  if (!CompilerDriver::target_machine) {
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    /// relocation model
    auto RM = llvm::Reloc::Model::PIC_;
    CompilerDriver::target_machine = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }
}

void CompilerDriver::emit_object(CompilationUnit *cu, const str &out_file) {
  auto *cg = _cg[cu];
  cg->emit_to_file(out_file);
}

Value *CompilerDriver::codegen(CompilationUnit *cu, bool print_ir) {
  TAN_ASSERT(_cg.find(cu) == _cg.end());
  auto *cg = _cg[cu] = new CodeGenerator(target_machine);
  auto *ret = cg->run(cu);

  if (print_ir) {
    cg->dump_ir();
  }

  return ret;
}

void CompilerDriver::dump_ast() const {
  // TODO: dump_ast()
}

void CompilerDriver::parse() {
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

void CompilerDriver::analyze() {
  for (auto *cu : _cu) {
    RegisterDeclarations rtld;
    rtld.run(cu);

    TypePrecheck tp;
    tp.run(cu);

    TypeCheck analyzer;
    analyzer.run(cu);
  }
}

TargetMachine *CompilerDriver::GetDefaultTargetMachine() {
  TAN_ASSERT(CompilerDriver::target_machine);
  return CompilerDriver::target_machine;
}

vector<str> CompilerDriver::resolve_import(const str &callee_path, const str &import_name) {
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
  /// search relative to directories in CompilerDriver::import_dirs
  for (const auto &rel : CompilerDriver::import_dirs) {
    auto p = fs::path(rel) / import_path;
    p = p.lexically_normal();
    if (fs::exists(p)) {
      ret.push_back(p.string());
    }
  }
  return ret;
}

const vector<CompilationUnit *> &CompilerDriver::get_compilation_units() const { return _cu; }
