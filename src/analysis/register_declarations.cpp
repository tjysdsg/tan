#include "analysis/register_declarations.h"
#include "ast/intrinsic.h"
#include "ast/stmt.h"

namespace tanlang {

RegisterDeclarations::RegisterDeclarations(SourceManager *sm) : AnalysisAction<RegisterDeclarations>(sm) {}

void RegisterDeclarations::run_impl(Program *p) { visit(p); }

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, Program) {
  push_scope(p);

  for (const auto &c : p->get_children()) {
    visit(c);
  }

  pop_scope();
}

// DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, Identifier)

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, Parenthesis) { visit(p->get_sub()); }

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, If) {
  size_t n = p->get_num_branches();
  for (size_t i = 0; i < n; ++i) {
    auto *cond = p->get_predicate(i);
    if (cond) { /// can be nullptr, meaning an "else" branch
      visit(cond);
    }

    visit(p->get_branch(i));
  }
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, VarDecl) {
  str name = p->get_name();
  if (ctx()->get_decl(name)) {
    error(p, fmt::format("Cannot redeclare variable named {}", name));
  }
  ctx()->set_decl(name, p);
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, ArgDecl) {
  str name = p->get_name();
  if (ctx()->get_decl(name)) {
    error(p, fmt::format("Cannot redeclare variable named {}", name));
  }
  ctx()->set_decl(name, p);
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, CompoundStmt) {
  push_scope(p);
  for (const auto &c : p->get_children()) {
    visit(c);
  }
  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, BinaryOrUnary) { visit(p->get_expr_ptr()); }

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, BinaryOperator) {
  visit(p->get_lhs());
  visit(p->get_rhs());
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, UnaryOperator) { visit(p->get_rhs()); }

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, Assignment) {
  visit(p->get_rhs());
  visit(p->get_lhs());
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, FunctionDecl) {
  top_ctx()->set_function_decl(p);
  push_scope(p);

  size_t n = p->get_n_args();
  const auto &arg_decls = p->get_arg_decls();
  for (size_t i = 0; i < n; ++i) {
    visit(arg_decls[i]);
  }

  if (!p->is_external()) {
    visit(p->get_body());
  }

  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, StructDecl) {
  // check if struct name is in conflicts of variable/function names
  str struct_name = p->get_name();
  if (!p->is_forward_decl()) {
    auto *root_ctx = top_ctx();
    auto *prev_decl = root_ctx->get_decl(struct_name);
    if (prev_decl && prev_decl != p) {
      if (!(prev_decl->get_node_type() == ASTNodeType::STRUCT_DECL &&
            ast_cast<StructDecl>(prev_decl)->is_forward_decl()))
        error(p, "Cannot redeclare type as a struct");
    }

    // overwrite the value set during parsing (e.g. forward decl)
    root_ctx->set_decl(struct_name, p);
  }
}

DEFINE_AST_VISITOR_IMPL(RegisterDeclarations, Loop) {
  push_scope(p);

  visit(p->get_predicate());
  visit(p->get_body());

  pop_scope();
}

} // namespace tanlang