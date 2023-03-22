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
#include <fmt/format.h>

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

void Compiler::emit_objects() {
  // FIXME: _cg->emit_to_file(filename);
}

Value *Compiler::codegen() {
  // FIXME:
  //  TAN_ASSERT(_parsed_module._program);
  //  TAN_ASSERT(!_cg);
  //  _cg = new CodeGenerator(_sm, target_machine);
  //  auto *ret = _cg->codegen(_parsed_module._program);
  //  return ret;
  return nullptr;
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
  TAN_ASSERT(!_parsed_modules.empty());

  for (const auto &p : _parsed_modules) {
    vector<Program *> sources(p.second.size());
    for (int i = 0; i < p.second.size(); ++i) {
      sources[i] = p.second[i]._program;
    }
    _packages[p.first] = Package::Create(p.first, sources);
  }
}

void Compiler::analyze() {
  merge_parsed_modules_by_package();

  // Register package-level declarations (struct, function, intrinsics, ...)
  // from all source files and imported packages in a symbol table.
  for (auto [package_name, package] : _packages) {
    Context *ctx = new Context(nullptr);
    for (auto *m : package->get_sources()) {
      if (!ctx->merge(*m->ctx())) {
        Error err(fmt::format("Name conflicts in {}", package_name));
        err.raise();
      }
    }

    _package_ctx[package_name] = ctx;

    // update top-level context of every source file
    for (auto *m : package->get_sources()) {
      m->set_ctx(ctx);
    }
  }

  // Type check these declarations as much as possible.
  // Meanwhile, build a dependency graph of unresolved type references, and sort topologically.
  TypeChecker type_checker;
  for (auto [package_name, package] : _packages) {
    type_checker.type_check(package, false, _package_ctx);
  }

  // Type check unresolved nodes using the sorted dependency graph and the symbol table
  // Make sure everything is resolved this time.
  for (auto [package_name, package] : _packages) {
    type_checker.type_check(package, true, _package_ctx);
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
