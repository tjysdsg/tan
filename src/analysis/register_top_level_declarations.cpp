#include "analysis/register_top_level_declarations.h"
#include "ast/stmt.h"

namespace tanlang {

RegisterTopLevelDeclarations::RegisterTopLevelDeclarations(SourceManager *sm)
    : AnalysisAction<RegisterTopLevelDeclarations>(sm) {}

void RegisterTopLevelDeclarations::run_impl(Program *p) {
  push_scope(p);

  for (const auto &c : p->get_children()) {
    if (c->get_node_type() == ASTNodeType::STRUCT_DECL) {
      CALL_AST_VISITOR(StructDecl, c);
    } else if (c->get_node_type() == ASTNodeType::FUNC_DECL) {
      CALL_AST_VISITOR(FunctionDecl, c);
    }
  }

  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(RegisterTopLevelDeclarations, FunctionDecl) { top_ctx()->add_function_decl(p); }

DEFINE_AST_VISITOR_IMPL(RegisterTopLevelDeclarations, StructDecl) {
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

} // namespace tanlang