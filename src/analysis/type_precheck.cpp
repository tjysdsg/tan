#include "analysis/type_precheck.h"
#include "ast/ast_base.h"
#include "ast/ast_node_type.h"
#include "common/ast_visitor.h"
#include "common/compilation_unit.h"
#include "ast/type.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "ast/context.h"
#include "source_file/token.h"
#include "compiler/compiler.h"
#include <iostream>
#include <set>

namespace tanlang {

void TypePrecheck::default_visit(ASTBase *) { TAN_ASSERT(false); }

void TypePrecheck::run_impl(CompilationUnit *cu) {
  _cu = cu;

  auto *p = cu->ast();

  push_scope(p);

  for (const auto &c : p->get_children()) {
    switch (c->get_node_type()) {
    case ASTNodeType::IMPORT:
      CALL_AST_VISITOR(Import, c);
      break;
    case ASTNodeType::STRUCT_DECL:
      CALL_AST_VISITOR(StructDecl, c);
      break;
    case ASTNodeType::FUNC_DECL:
      CALL_AST_VISITOR(FunctionDecl, c);
      break;
    case ASTNodeType::INTRINSIC:
      CALL_AST_VISITOR(Intrinsic, c);
      break;
    default:
      break;
    }
  }

  pop_scope();
}

Type *TypePrecheck::check_type_ref(Type *p, ASTBase *node) {
  TAN_ASSERT(p->is_ref());
  Type *ret = p;

  const str &referred_name = p->get_typename();
  auto *decl = search_decl_in_scopes(referred_name);
  if (decl && decl->is_type_decl()) {
    ret = decl->get_type();
    TAN_ASSERT(ret);
    if (!ret->is_canonical()) {
      _cu->top_level_symbol_dependency.add_dependency(decl, node);
    }
  } else {
    error(ErrorType::TYPE_ERROR, node, fmt::format("Unknown type {}", referred_name));
  }

  return ret;
}

Type *TypePrecheck::check_type(Type *p, ASTBase *node) {
  TAN_ASSERT(p);
  TAN_ASSERT(node);

  Type *ret = p;
  if (p->is_ref()) {
    ret = check_type_ref(p, node);
  } else if (p->is_pointer()) {
    auto *pointee = ((PointerType *)p)->get_pointee();

    TAN_ASSERT(pointee);
    if (pointee->is_ref()) {
      pointee = check_type_ref(pointee, node);
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
    error(ErrorType::IMPORT_ERROR, p, "Cannot import: " + file);
  }

  auto *compiler = new Compiler(imported);
  compiler->parse();
  compiler->analyze();
  Context *imported_ctx = compiler->get_compilation_units()[0]->ast()->ctx();

  // import functions
  vector<FunctionDecl *> funcs = imported_ctx->get_func_decls();
  vector<FunctionDecl *> pub_funcs{};
  for (auto *f : funcs) {
    f->set_start(p->start()); // FIXME[HACK]: source location of imported function is not usable in current file
    f->set_end(p->end());

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

/*
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
*/

DEFINE_AST_VISITOR_IMPL(TypePrecheck, Intrinsic) {
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

DEFINE_AST_VISITOR_IMPL(TypePrecheck, VarDecl) {
  Type *ty = p->get_type();
  if (ty) {
    p->set_type(check_type(ty, p));
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, ArgDecl) { p->set_type(check_type(p->get_type(), p)); }

DEFINE_AST_VISITOR_IMPL(TypePrecheck, FunctionDecl) {
  push_scope(p);

  /// update return type
  auto *func_type = (FunctionType *)p->get_type();
  auto *ret_type = check_type(func_type->get_return_type(), p);
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
      _cu->top_level_symbol_dependency.add_dependency(m, p);
    }

    if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
      str name = ast_cast<VarDecl>(m)->get_name();
      p->set_member_index(name, (int)i);
      (*ty)[i] = m->get_type();
    } else if (m->get_node_type() == ASTNodeType::ASSIGN) { /// member variable with an initial value
      auto bm = ast_cast<Assignment>(m);

      if (bm->get_lhs()->get_node_type() != ASTNodeType::VAR_DECL) {
        error(ErrorType::SEMANTIC_ERROR, bm, "Expect a member variable declaration");
      }
      auto decl = ast_cast<VarDecl>(bm->get_lhs());

      (*ty)[i] = decl->get_type();
      p->set_member_index(decl->get_name(), (int)i);
    } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) { /// member functions
      auto f = ast_cast<FunctionDecl>(m);

      (*ty)[i] = f->get_type();
      p->set_member_index(f->get_name(), (int)i);
    } else {
      error(ErrorType::SEMANTIC_ERROR, p, "Invalid struct member");
    }
  }

  pop_scope();
}

} // namespace tanlang
