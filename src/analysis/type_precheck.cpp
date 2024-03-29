#include "analysis/type_precheck.h"
#include "ast/ast_base.h"
#include "ast/ast_node_type.h"
#include "common/ast_visitor.h"
#include "ast/type.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/package.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "ast/context.h"
#include "source_file/token.h"
#include "driver/driver.h"
#include <iostream>

namespace tanlang {

void TypePrecheck::default_visit(ASTBase *) { TAN_ASSERT(false); }

void TypePrecheck::run_impl(Package *p) {
  _package = p;

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
      _package->top_level_symbol_dependency.add_dependency(decl, node);
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
    auto *pointee = pcast<PointerType>(p)->get_pointee();

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

DEFINE_AST_VISITOR_IMPL(TypePrecheck, Import) {
  str name = p->get_name();

  auto *compiler = CompilerDriver::instance();
  TAN_ASSERT(compiler);
  Package *package = compiler->get_package(name);
  if (!package) {
    error(ErrorType::IMPORT_ERROR, p, "Cannot find package named: " + name);
  }

  Context *imported_ctx = package->ctx();

  // Imported declarations are stored in both the package's context and this import AST node:
  // 1. The context only serves as a search table
  // 2. The type checker and code generator will operate on the declarations stored in the import node

  // import functions
  vector<FunctionDecl *> funcs = imported_ctx->get_func_decls();
  for (auto *f : funcs) {
    f->set_start(p->start());
    f->set_end(p->end());

    if (f->is_public() || f->is_external()) {
      // FIXME: merge multiple declarations of the same symbol, fail if they don't match
      //        for example, intrinsics
      auto *existing = top_ctx()->get_func_decl(f->get_name());
      if (!existing) {
        p->_imported_funcs.push_back(f);
        top_ctx()->set_function_decl(f);
      }
    }
  }

  // imported types
  vector<Decl *> decls = imported_ctx->get_decls();
  for (auto *t : decls) {
    if (t->is_type_decl() && t->is_public()) {
      top_ctx()->set_decl(t->get_name(), t);
      p->_imported_types.push_back(pcast<TypeDecl>(t));
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
    auto *tce = pcast<TestCompError>(p->get_sub());
    if (tce->_caught)
      return;

    push_scope(p);

    try {
      for (auto *c : tce->get_children())
        visit(c);
    } catch (const CompileException &e) {
      std::cerr << fmt::format("Caught expected compile error: {}\nContinue compilation...\n", e.what());
      tce->_caught = true;
    }

    pop_scope();
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, VarDecl) {
  Type *ty = p->get_type();
  if (ty) {
    p->set_type(check_type(ty, p));
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, ArgDecl) { p->set_type(check_type(p->get_type(), p)); }

DEFINE_AST_VISITOR_IMPL(TypePrecheck, Assignment) {
  auto *lhs = p->get_lhs();
  visit(lhs);

  // at this stage, we find out the type of assignment only if it's specified
  if (lhs->get_node_type() == ASTNodeType::VAR_DECL) {
    p->set_type(pcast<Decl>(lhs)->get_type());
  }
}

DEFINE_AST_VISITOR_IMPL(TypePrecheck, FunctionDecl) {
  push_scope(p);

  /// update return type
  auto *func_type = pcast<FunctionType>(p->get_type());
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
  auto *ty = pcast<StructType>(p->get_type());
  TAN_ASSERT(ty);

  push_scope(p);

  for (size_t i = 0; i < n; ++i) {
    Expr *m = members[i];

    // Check if m is a struct member declaration before visiting
    switch (m->get_node_type()) {
    case ASTNodeType::VAR_DECL:
    case ASTNodeType::FUNC_DECL:
      break;
    case ASTNodeType::ASSIGN: {
      auto *assign = pcast<Assignment>(m);
      if (assign->get_lhs()->get_node_type() != ASTNodeType::VAR_DECL)
        error(ErrorType::SEMANTIC_ERROR, assign, "Expect a member variable declaration");
      break;
    }
    default:
      error(ErrorType::SEMANTIC_ERROR, p, "Invalid struct member");
    }

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
      _package->top_level_symbol_dependency.add_dependency(m, p);
    }

    if (m->get_node_type() == ASTNodeType::VAR_DECL) { // member variable without initial value
      str name = pcast<VarDecl>(m)->get_name();
      p->set_member_index(name, (int)i);
      (*ty)[i] = m->get_type();

    } else if (m->get_node_type() == ASTNodeType::ASSIGN) { // member variable with an initial value
      auto *assign = pcast<Assignment>(m);
      auto decl = pcast<VarDecl>(assign->get_lhs());

      (*ty)[i] = decl->get_type();

      // member name -> index
      p->set_member_index(decl->get_name(), (int)i);

      // set rhs of assignment as the default value of this member
      auto *rhs = assign->get_rhs();
      if (!rhs->is_comptime_known()) {
        error(ErrorType::SEMANTIC_ERROR, rhs, "Expect the value to be compile time known");
      }
      p->set_member_default_val((int)i, rhs);

    } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) { // member functions
      auto f = pcast<FunctionDecl>(m);

      (*ty)[i] = f->get_type();
      p->set_member_index(f->get_name(), (int)i);
    } else {
      TAN_ASSERT(false);
    }
  }

  pop_scope();
}

} // namespace tanlang
