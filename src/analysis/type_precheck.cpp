#include "analysis/type_precheck.h"
#include "ast/ast_base.h"
#include "common/ast_visitor.h"
#include "ast/type.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "ast/context.h"
#include "lexer/token.h"
#include "compiler/compiler.h"
#include <iostream>
#include <csetjmp>
#include <set>

namespace tanlang {

void TypePrecheck::default_visit(ASTBase *) { TAN_ASSERT(false); }

TypePrecheck::TypePrecheck(SourceManager *sm) : AnalysisAction<TypePrecheck>(sm) { _sm = sm; }

vector<ASTBase *> TypePrecheck::sorted_unresolved_symbols() const { return _unresolved_symbols.topological_sort(); }

void TypePrecheck::run_impl(Program *p) {
  push_scope(p);

  for (const auto &c : p->get_children()) {
    if (c->get_node_type() == ASTNodeType::IMPORT) {
      CALL_AST_VISITOR(Import, c);
    } else if (c->get_node_type() == ASTNodeType::STRUCT_DECL) {
      CALL_AST_VISITOR(StructDecl, c);
    } else if (c->get_node_type() == ASTNodeType::FUNC_DECL) {
      CALL_AST_VISITOR(FunctionDecl, c);
    }
  }

  pop_scope();
}

Type *TypePrecheck::check_type_ref(Type *p, SrcLoc loc, ASTBase *node) {
  TAN_ASSERT(p->is_ref());
  Type *ret = p;

  const str &referred_name = p->get_typename();
  auto *decl = search_decl_in_scopes(referred_name);
  if (decl && decl->is_type_decl()) {
    ret = decl->get_type();
    TAN_ASSERT(ret);
    if (!ret->is_canonical()) {
      _unresolved_symbols.add_dependency(decl, node);
    }
  } else {
    Error err(_sm->get_filename(), _sm->get_token(loc), fmt::format("Unknown type {}", referred_name));
    err.raise();
  }

  return ret;
}

Type *TypePrecheck::check_type(Type *p, SrcLoc loc, ASTBase *node) {
  TAN_ASSERT(p);
  TAN_ASSERT(node);

  Type *ret = p;
  if (p->is_ref()) {
    ret = check_type_ref(p, loc, node);
  } else if (p->is_pointer()) {
    auto *pointee = ((PointerType *)p)->get_pointee();

    TAN_ASSERT(pointee);
    if (pointee->is_ref()) {
      pointee = check_type_ref(pointee, loc, node);
      if (pointee->is_canonical()) {
        ret = Type::GetPointerType(pointee);
      }
    }
  }

  TAN_ASSERT(ret);
  return ret;
}

// TODO: check recursive import
DEFINE_AST_VISITOR_IMPL(TypePrecheck, Import) {
  str file = p->get_filename();
  auto imported = Compiler::resolve_import(_sm->get_filename(), file);
  if (imported.empty()) {
    error(p, "Cannot import: " + file);
  }

  auto *compiler = new Compiler(imported[0]);
  compiler->parse();
  compiler->analyze();
  Context *imported_ctx = compiler->get_root_ast()->ctx();

  // import functions
  vector<FunctionDecl *> funcs = imported_ctx->get_func_decls();
  vector<FunctionDecl *> pub_funcs{};
  for (auto *f : funcs) {
    f->set_loc(p->loc()); // FIXME[HACK]: source location of imported function is not usable in current file
    if (f->is_public() || f->is_external()) {
      auto *existing = top_ctx()->get_func_decl(f->get_name());
      if (!existing) {
        // TODO: merge multiple declarations of the same symbol, fail if they don't match
        pub_funcs.push_back(f);
        top_ctx()->set_function_decl(f);
      }
    }
  }
  p->set_imported_funcs(pub_funcs);

  // import type declarations
  // TODO: distinguish local and global type decls
  vector<Decl *> decls = imported_ctx->get_decls();
  vector<TypeDecl *> type_decls{};
  for (auto *t : decls) {
    if (t->is_type_decl()) {
      top_ctx()->set_decl(t->get_name(), t);
    }
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, Identifier) {
  auto *referred = search_decl_in_scopes(p->get_name());
  if (referred) {
    if (referred->is_type_decl()) { /// refers to a type
      auto *ty = check_type_ref(referred->get_type(), p->loc(), p);
      p->set_type_ref(ty);
    } else { /// refers to a variable
      p->set_var_ref(VarRef::Create(p->loc(), p->get_name(), referred));
      p->set_type(check_type(referred->get_type(), p->loc(), p));
    }
  } else {
    error(p, "Unknown identifier");
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, VarDecl) {
  Type *ty = p->get_type();
  if (ty) {
    p->set_type(check_type(ty, p->loc(), p));
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, ArgDecl) { p->set_type(check_type(p->get_type(), p->loc(), p)); }

DEFINE_AST_VISITOR_IMPL(TypePrecheck, FunctionDecl) {
  push_scope(p);

  /// update return type
  auto *func_type = (FunctionType *)p->get_type();
  auto *ret_type = check_type(func_type->get_return_type(), p->loc(), p);
  func_type->set_return_type(ret_type);

  /// type_check_ast args
  size_t n = p->get_n_args();
  const auto &arg_decls = p->get_arg_decls();
  vector<Type *> arg_types(n, nullptr);
  for (size_t i = 0; i < n; ++i) {
    visit(arg_decls[i]); /// args will be added to the scope here
    arg_types[i] = arg_decls[i]->get_type();
  }
  func_type->set_arg_types(arg_types); /// update arg types

  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, StructDecl) {
  str struct_name = p->get_name();

  auto members = p->get_member_decls();
  size_t n = members.size();
  auto *ty = (StructType *)p->get_type();
  TAN_ASSERT(ty);

  push_scope(p);

  for (size_t i = 0; i < n; ++i) {
    Expr *m = members[i];
    visit(m);

    /*
     * DO NOT add unresolved symbol dependency if m's type is a pointer to an unresolved type reference
     * This allows us to define a struct that holds a pointer to itself, like LinkedList.
     *
     * This works because:
     * 1. m is registered in the unresolved symbol dependency graph so it will be re-analyzed in
     *    the final analysis stage.
     * 2. Nothing actually directly relies on the type of m. For example, size in bits is always the size of a
     *    pointer.
     */
    if (!m->get_type()->is_canonical() && !m->get_type()->is_pointer()) {
      _unresolved_symbols.add_dependency(m, p);
    }

    if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
      str name = ast_cast<VarDecl>(m)->get_name();
      p->set_member_index(name, i);
      ty->append_member_type(m->get_type());
    } else if (m->get_node_type() == ASTNodeType::ASSIGN) { /// member variable with an initial value
      auto bm = ast_cast<Assignment>(m);

      if (bm->get_lhs()->get_node_type() != ASTNodeType::VAR_DECL) {
        error(bm, "Expect a member variable declaration");
      }
      auto decl = ast_cast<VarDecl>(bm->get_lhs());

      ty->append_member_type(decl->get_type());
      p->set_member_index(decl->get_name(), i);
    } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) { /// member functions
      auto f = ast_cast<FunctionDecl>(m);

      ty->append_member_type(f->get_type());
      p->set_member_index(f->get_name(), i);
    } else {
      error(p, "Invalid struct member");
    }
  }

  pop_scope();
}

} // namespace tanlang
