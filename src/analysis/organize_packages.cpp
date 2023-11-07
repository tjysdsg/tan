#include <iostream>
#include "analysis/organize_packages.h"
#include "common/compilation_unit.h"
#include "ast/package.h"
#include "ast/context.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"

using namespace tanlang;

vector<Package *> OrganizePackages::run_impl(vector<CompilationUnit *> cu) {
  // reset
  _package_top_level_ctx.clear();
  _package_top_level_asts.clear();

  // visit top-level ASTs and remember top-level ASTs
  for (auto *c : cu) {
    auto *ast = c->ast();
    visit(ast);
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

  if (package_name.empty()) { // default package name to "main" if not specified
    // Error(ErrorType::SEMANTIC_ERROR, fmt::format("Missing a package stmt in {}", p->src()->get_filename())).raise();
    package_name = "main";
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
