#include "compiler/compiler.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "analysis/type_checker.h"
#include "codegen/code_generator.h"
#include "ast/intrinsic.h"
#include "ast/stmt.h"
#include "ast/package.h"
#include "ast/context.h"
#include "lexer/reader.h"
#include "parser/parser.h"
#include <filesystem>

using namespace tanlang;
namespace fs = std::filesystem;

Compiler::Compiler() {
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

void Compiler::emit_objects() { _cg->emit_to_file(filename); }

Value *Compiler::codegen() {
  TAN_ASSERT(_parsed_module._program);
  TAN_ASSERT(!_cg);
  _cg = new CodeGenerator(_sm, target_machine);
  auto *ret = _cg->codegen(_parsed_module._program);
  return ret;
}

void Compiler::dump_ir() const {
  TAN_ASSERT(_cg);
  _cg->dump_ir();
}

void Compiler::dump_ast() const {
  for (const auto &ps : _parsed_modules) {
    for (const auto &p : ps.second) {
      p._program->printTree();
    }
  }
}

void Compiler::parse(const str &filename) {
  Reader reader;
  reader.open(filename);

  auto tokens = tokenize(&reader);
  auto *sm = new SourceManager(filename, std::move(tokens));

  auto *parser = new Parser(sm);
  ParsedModule p = parser->parse();
  _parsed_modules[p._package_name].push_back(p);
}

void Compiler::merge_parsed_modules_by_package() {
  for (const auto &p : _parsed_modules) {
    vector<Program *> sources(p.second.size());
    for (int i = 0; i < p.second.size(); ++i) {
      sources[i] = p.second[i]._program;
    }
    _packages[p.first] = Package::Create(p.first, sources);
  }
}

void Compiler::analyze() {
  TypeChecker type_checker;
  umap<ASTBase *, SourceManager *> source_managers{};

  // Register package-level declarations (struct, function, intrinsics, ...)
  // from all source files and imported packages in a symbol table.

  // Type check these declarations as much as possible.
  // Meanwhile, build a dependency graph of type references, and sort topologically.

  // Type check package-level declarations again using the sorted dependency graph and the symbol table,
  // skip what we already checked. Make sure everything is resolved this time.
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
