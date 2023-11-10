#include <iostream>
#include "analysis/organize_packages.h"
#include "ast/package.h"
#include "ast/context.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"

#include <filesystem>
namespace fs = std::filesystem;

using namespace tanlang;

vector<Package *> OrganizePackages::run_impl(vector<Program *> ps) {
  // reset
  _package_top_level_ctx.clear();
  _package_top_level_asts.clear();

  // visit top-level ASTs and remember top-level ASTs
  for (auto *p : ps) {
    visit(p);
  }

  vector<Package *> ret{};
  for (auto [name, asts] : _package_top_level_asts) {
    auto *package = new Package(name, asts);

    // merge contexts from multiple Program's to get Package-level context
    vector<Context *> ctx = _package_top_level_ctx[name];
    auto *package_ctx = package->ctx();
    for (Context *c : ctx) {
      for (auto *d : c->get_decls()) {
        package_ctx->set_decl(d->get_name(), d);
      }
      for (auto *d : c->get_func_decls()) {
        package_ctx->set_function_decl(d);
      }
    }

    ret.push_back(package);
  }

  return ret;
}

DEFINE_AST_VISITOR_IMPL(OrganizePackages, Program) {
  // get a list of top-level ASTs
  vector<ASTBase *> asts{};
  str package_name;
  for (auto *n : p->get_children()) {
    if (n->get_node_type() == ASTNodeType::PACKAGE_DECL) {

      if (!package_name.empty()) { // repeated declaration
        Error(ErrorType::SEMANTIC_ERROR, fmt::format("Can only have one package stmt in {}", p->src()->get_filename()))
            .raise();
      }

      package_name = pcast<PackageDecl>(n)->get_name();
    } else {
      visit(n);
      asts.push_back(n);
    }
  }

  if (package_name.empty()) { // default package name
    package_name = fs::path(p->src()->get_filename()).filename().replace_extension().string();

    if (package_name.empty()) { // ".tan" is invalid
      Error(ErrorType::GENERIC_ERROR,
            fmt::format("Cannot deduce default package name for {}", p->src()->get_filename()))
          .raise();
    }
  }

  // register ASTs
  {
    auto q = _package_top_level_asts.find(package_name);
    if (q == _package_top_level_asts.end()) {
      _package_top_level_asts[package_name] = asts;
    } else {
      q->second.insert(q->second.end(), asts.begin(), asts.end());
    }
  }

  // register context
  {
    auto q = _package_top_level_ctx.find(package_name);
    if (q == _package_top_level_ctx.end()) {
      _package_top_level_ctx[package_name] = vector<Context *>{p->ctx()};
    } else {
      q->second.push_back(p->ctx());
    }
  }
}

DEFINE_AST_VISITOR_IMPL(OrganizePackages, Intrinsic) {
  // check children if this is @test_comp_error
  if (p->get_intrinsic_type() == IntrinsicType::TEST_COMP_ERROR) {

    try {
      auto *sub = p->get_sub();
      if (sub) {
        TAN_ASSERT(sub->get_node_type() == ASTNodeType::COMPOUND_STATEMENT);
        for (auto *c : sub->get_children())
          visit(c);
      }
    } catch (const CompileException &e) {
      std::cerr << fmt::format("Caught expected compile error: {}\nContinue compilation...\n", e.what());
      p->set_sub(nullptr); // no need to check again in later stages
    }
  }
}
