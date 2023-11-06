#include "analysis/type_check.h"
#include "ast/ast_base.h"
#include "ast/ast_node_type.h"
#include "common/ast_visitor.h"
#include "ast/type.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "ast/context.h"
#include "fmt/core.h"
#include "source_file/token.h"
#include <iostream>
#include <set>

namespace tanlang {

void TypeCheck::run_impl(Package *p) {
  push_scope(p);

  auto sorted_top_level_decls = p->top_level_symbol_dependency.topological_sort();

  // std::cout << "Sorted unresolved symbol dependency:\n";
  // for (auto *d : sorted_top_level_decls) {
  //   str name = pcast<Decl>(d)->get_name();
  //   std::cout << name << '\n';
  // }

  for (auto *c : sorted_top_level_decls) {
    visit(c);
  }

  std::set<ASTBase *> s(sorted_top_level_decls.begin(), sorted_top_level_decls.end());
  for (auto *c : p->get_children()) {
    if (!s.contains(c))
      visit(c);
  }

  pop_scope();
}

FunctionDecl *TypeCheck::search_function_callee(FunctionCall *p) {
  const str &name = p->get_name();
  const vector<Expr *> &args = p->_args;

  FunctionDecl *candidate = top_ctx()->get_func_decl(name);
  if (!candidate) {
    error(ErrorType::TYPE_ERROR, p, fmt::format("Unknown function call: {}", name));
  }

  size_t n = candidate->get_n_args();
  if (n != args.size()) {
    error(ErrorType::SEMANTIC_ERROR, p,
          fmt::format("Incorrect number of arguments: expect {} but found {}", candidate->get_n_args(), n));
  }

  auto *func_type = pcast<FunctionType>(candidate->get_type());

  // Check if argument types match (return type not checked)
  // Allow implicit cast from actual arguments to expected arguments
  for (size_t i = 0; i < n; ++i) {
    auto *actual_type = args[i]->get_type();
    auto *expected_type = func_type->get_arg_types()[i];

    if (actual_type != expected_type) {
      if (!CanImplicitlyConvert(actual_type, expected_type)) {
        error(ErrorType::TYPE_ERROR, p,
              fmt::format("Cannot implicitly convert the type of argument {}: expect {} but found {}", i + 1,
                          actual_type->get_typename(), expected_type->get_typename()));
      }
    }
  }

  return candidate;
}

Type *TypeCheck::resolve_type_ref(Type *p, ASTBase *node) {
  TAN_ASSERT(p->is_ref());
  Type *ret = p;

  const str &referred_name = p->get_typename();
  auto *decl = search_decl_in_scopes(referred_name);
  if (decl && decl->is_type_decl()) {
    ret = decl->get_type();
  } else {
    error(ErrorType::TYPE_ERROR, node, fmt::format("Unknown type {}", referred_name));
  }

  TAN_ASSERT(ret);
  return ret;
}

Type *TypeCheck::resolve_type(Type *p, ASTBase *node) {
  TAN_ASSERT(p);

  Type *ret = p;
  if (p->is_ref()) {
    ret = resolve_type_ref(p, node);
  } else if (p->is_pointer()) {
    auto *pointee = pcast<PointerType>(p)->get_pointee();

    TAN_ASSERT(pointee);
    if (pointee->is_ref()) {
      pointee = resolve_type_ref(pointee, node);
      ret = Type::GetPointerType(pointee);
    }
  }

  TAN_ASSERT(ret);
  return ret;
}

void TypeCheck::analyze_func_decl_prototype(ASTBase *_p) {
  auto *p = pcast<FunctionDecl>(_p);

  push_scope(p);

  /// update return type
  auto *func_type = pcast<FunctionType>(p->get_type());
  auto *ret_type = resolve_type(func_type->get_return_type(), p);
  func_type->set_return_type(ret_type);

  /// type_check_ast args
  size_t n = p->get_n_args();
  const auto &arg_decls = p->get_arg_decls();
  vector<Type *> arg_types(n, nullptr);
  for (size_t i = 0; i < n; ++i) {
    visit(arg_decls[i]); /// args will be added to the scope here
    arg_types[i] = arg_decls[i]->get_type();
    TAN_ASSERT(arg_types[i]->is_canonical());
  }
  func_type->set_arg_types(arg_types); /// update arg types

  pop_scope();
}

void TypeCheck::analyze_func_body(ASTBase *_p) {
  auto *p = pcast<FunctionDecl>(_p);

  push_scope(p);

  if (!p->is_external()) {
    visit(p->get_body());
  }

  pop_scope();
}

void TypeCheck::analyze_function_call(FunctionCall *p, bool include_intrinsics) {
  for (const auto &a : p->_args) {
    visit(a);
  }

  FunctionDecl *callee = search_function_callee(p);
  if (include_intrinsics || !callee->is_intrinsic()) {
    p->_callee = callee;
  } else {
    error(ErrorType::UNKNOWN_SYMBOL, p,
          fmt::format("Unknown function call. Maybe use @{} if you want to call this intrinsic?", p->get_name()));
  }

  auto *func_type = pcast<FunctionType>(callee->get_type());
  p->set_type(func_type->get_return_type());
}

void TypeCheck::analyze_intrinsic_func_call(Intrinsic *p, FunctionCall *func_call) {
  auto *void_type = Type::GetVoidType();
  switch (p->get_intrinsic_type()) {
  case IntrinsicType::STACK_TRACE:
    func_call->set_name(Intrinsic::STACK_TRACE_FUNCTION_REAL_NAME);
    [[fallthrough]];
  case IntrinsicType::ABORT:
    analyze_function_call(func_call, true);
    p->set_type(void_type);
    break;
  case IntrinsicType::GET_DECL: {
    if (func_call->get_n_args() != 1) {
      error(ErrorType::SEMANTIC_ERROR, func_call, "Expect the number of args to be 1");
    }

    auto *target = func_call->get_arg(0);
    if (target->get_node_type() != ASTNodeType::ID) {
      error(ErrorType::TYPE_ERROR, target,
            fmt::format("Expect an identifier as the operand, but got {}",
                        ASTBase::ASTTypeNames[target->get_node_type()]));
    }

    visit(target);

    auto *id = pcast<Identifier>(target);
    if (id->get_id_type() != IdentifierType::ID_VAR_REF) {
      error(ErrorType::TYPE_ERROR, id, fmt::format("Expect a value but got type"));
    }

    auto *decl = id->get_var_ref()->get_referred();
    auto *source_str = Literal::CreateStringLiteral(p->src(), _sm->get_source_code(decl->start(), decl->end()));

    // FEATURE: Return AST?
    p->set_sub(source_str);
    p->set_type(source_str->get_type());
    break;
  }
  case IntrinsicType::COMP_PRINT: {
    p->set_type(void_type);

    // FEATURE: print with var args
    auto args = func_call->_args;
    if (args.size() != 1 || args[0]->get_node_type() != ASTNodeType::STRING_LITERAL) {
      error(ErrorType::TYPE_ERROR, p, "Invalid call to compprint, one argument with type 'str' required");
    }

    str msg = pcast<StringLiteral>(args[0])->get_value();
    std::cout << fmt::format("Message ({}): {}\n", _sm->get_src_location_str(p->start()), msg);
    break;
  }
  default:
    TAN_ASSERT(false);
  }
}

// ASSUMES lhs has been already analyzed, while rhs has not
void TypeCheck::analyze_member_func_call(MemberAccess *p, Expr *lhs, FunctionCall *rhs) {
  if (!lhs->is_lvalue() && !lhs->get_type()->is_pointer()) {
    error(ErrorType::TYPE_ERROR, p, "Invalid member function call");
  }

  // insert the address of the struct instance as the first parameter
  if (lhs->is_lvalue() && !lhs->get_type()->is_pointer()) {
    Expr *tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs->src(), lhs);
    visit(tmp);
    rhs->_args.insert(rhs->_args.begin(), tmp);
  } else {
    rhs->_args.insert(rhs->_args.begin(), lhs);
  }

  visit(rhs);
  p->set_type(rhs->get_type());
}

// ASSUMES lhs has been already analyzed, while rhs has not
void TypeCheck::analyze_bracket_access(MemberAccess *p, Expr *lhs, Expr *rhs) {
  visit(rhs);

  if (!lhs->is_lvalue()) {
    error(ErrorType::TYPE_ERROR, p, "Expect lhs to be an lvalue");
  }

  auto *lhs_type = lhs->get_type();
  if (!(lhs_type->is_pointer() || lhs_type->is_array() || lhs_type->is_string())) {
    error(ErrorType::TYPE_ERROR, p, "Expect a type that supports bracket access");
  }
  if (!rhs->get_type()->is_int()) {
    error(ErrorType::TYPE_ERROR, rhs, "Expect an integer");
  }

  Type *sub_type = nullptr;
  if (lhs_type->is_pointer()) {
    sub_type = pcast<PointerType>(lhs_type)->get_pointee();
  } else if (lhs_type->is_array()) {
    auto *array_type = pcast<ArrayType>(lhs_type);
    sub_type = array_type->get_element_type();
    /// check if array index is out-of-bound
    if (rhs->get_node_type() == ASTNodeType::INTEGER_LITERAL) {
      uint64_t size = pcast<IntegerLiteral>(rhs)->get_value();
      if (lhs->get_type()->is_array() && (int)size >= array_type->array_size()) {
        error(ErrorType::TYPE_ERROR, p,
              fmt::format("Index {} out of bound, the array size is {}", std::to_string(size),
                          std::to_string(array_type->array_size())));
      }
    }
  } else if (lhs_type->is_string()) {
    sub_type = Type::GetCharType();
  }

  p->set_type(sub_type);
}

// ASSUMES lhs has been already analyzed, while rhs has not
void TypeCheck::analyze_member_access_member_variable(MemberAccess *p, Expr *lhs, Expr *rhs) {
  str m_name = pcast<Identifier>(rhs)->get_name();
  Type *struct_ty = nullptr;
  /// auto dereference pointers
  if (lhs->get_type()->is_pointer()) {
    struct_ty = pcast<PointerType>(lhs->get_type())->get_pointee();
  } else {
    struct_ty = lhs->get_type();
  }

  struct_ty = resolve_type(struct_ty, lhs);
  if (!struct_ty->is_struct()) {
    error(ErrorType::TYPE_ERROR, lhs, "Expect a struct type");
  }

  auto *struct_decl = pcast<StructDecl>(search_decl_in_scopes(struct_ty->get_typename()));
  p->_access_idx = struct_decl->get_struct_member_index(m_name);
  if (p->_access_idx == -1) {
    error(ErrorType::UNKNOWN_SYMBOL, p,
          fmt::format("Cannot find member variable '{}' of struct '{}'", m_name, struct_decl->get_name()));
  }
  auto *ty = struct_decl->get_struct_member_ty(p->_access_idx);
  p->set_type(resolve_type(ty, p));
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, Identifier) {
  auto *referred = search_decl_in_scopes(p->get_name());
  if (referred) {
    if (referred->is_type_decl()) { /// refers to a type
      auto *ty = resolve_type_ref(referred->get_type(), p);
      p->set_type_ref(ty);
    } else { /// refers to a variable
      p->set_var_ref(VarRef::Create(p->src(), p->get_name(), referred));
      p->set_type(resolve_type(referred->get_type(), p));
    }
  } else {
    error(ErrorType::UNKNOWN_SYMBOL, p, "Unknown identifier");
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, Parenthesis) {
  visit(p->get_sub());
  p->set_type(p->get_sub()->get_type());
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, If) {
  size_t n = p->get_num_branches();
  for (size_t i = 0; i < n; ++i) {
    auto *cond = p->get_predicate(i);
    if (cond) { /// can be nullptr, meaning an "else" branch
      visit(cond);
      p->set_predicate(i, create_implicit_conversion(cond, Type::GetBoolType()));
    }

    visit(p->get_branch(i));
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, VarDecl) {
  Type *ty = p->get_type();

  // assume the type is always non-null
  // type_check_assignment is responsible for setting the deduced type if necessary
  if (!ty) {
    error(ErrorType::TYPE_ERROR, p, "Cannot deduce the type of variable declaration");
  }
  p->set_type(resolve_type(ty, p));
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, ArgDecl) { p->set_type(resolve_type(p->get_type(), p)); }

DEFINE_AST_VISITOR_IMPL(TypeCheck, Return) {
  FunctionDecl *func = search_node_in_parent_scopes<FunctionDecl, ASTNodeType::FUNC_DECL>();
  if (!func) {
    error(ErrorType::TYPE_ERROR, p, "Return statement must be inside a function definition");
  }

  auto *rhs = p->get_rhs();
  Type *ret_type = Type::GetVoidType();
  if (rhs) {
    visit(rhs);
    ret_type = rhs->get_type();
  }
  // check if return type is the same as the function return type
  if (!CanImplicitlyConvert(ret_type, pcast<FunctionType>(func->get_type())->get_return_type())) {
    error(ErrorType::TYPE_ERROR, p, "Returned type cannot be coerced to function return type");
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, CompoundStmt) {
  push_scope(p);

  for (const auto &c : p->get_children()) {
    visit(c);
  }

  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, BinaryOrUnary) { visit(p->get_expr_ptr()); }

DEFINE_AST_VISITOR_IMPL(TypeCheck, BinaryOperator) {
  Expr *lhs = p->get_lhs();
  Expr *rhs = p->get_rhs();

  if (p->get_op() == BinaryOpKind::MEMBER_ACCESS) {
    CALL_AST_VISITOR(MemberAccess, p);
    return;
  }

  visit(lhs);
  visit(rhs);

  switch (p->get_op()) {
  case BinaryOpKind::SUM:
  case BinaryOpKind::SUBTRACT:
  case BinaryOpKind::MULTIPLY:
  case BinaryOpKind::DIVIDE:
  case BinaryOpKind::BAND:
  case BinaryOpKind::BOR:
  case BinaryOpKind::MOD: {
    p->set_type(auto_promote_bop_operand_types(p));
    break;
  }
  case BinaryOpKind::LAND:
  case BinaryOpKind::LOR:
  case BinaryOpKind::XOR: {
    // check if both operators are bool
    auto *bool_type = PrimitiveType::GetBoolType();
    p->set_lhs(create_implicit_conversion(lhs, bool_type));
    p->set_rhs(create_implicit_conversion(rhs, bool_type));
    p->set_type(bool_type);
    break;
  }
  case BinaryOpKind::GT:
  case BinaryOpKind::GE:
  case BinaryOpKind::LT:
  case BinaryOpKind::LE:
  case BinaryOpKind::EQ:
  case BinaryOpKind::NE:
    auto_promote_bop_operand_types(p);
    p->set_type(PrimitiveType::GetBoolType());
    break;
  default:
    TAN_ASSERT(false);
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, UnaryOperator) {
  auto *rhs = p->get_rhs();
  visit(rhs);

  auto *rhs_type = rhs->get_type();
  switch (p->get_op()) {
  case UnaryOpKind::LNOT:
    rhs = create_implicit_conversion(rhs, Type::GetBoolType());
    p->set_rhs(rhs);
    p->set_type(PrimitiveType::GetBoolType());
    break;
  case UnaryOpKind::BNOT:
    if (!rhs_type->is_int()) {
      error(ErrorType::TYPE_ERROR, rhs, "Expect an integer type");
    }
    p->set_type(rhs_type);
    break;
  case UnaryOpKind::ADDRESS_OF:
    p->set_type(Type::GetPointerType(rhs_type));
    break;
  case UnaryOpKind::PTR_DEREF:
    if (!rhs_type->is_pointer()) {
      error(ErrorType::TYPE_ERROR, rhs, "Expect a pointer type");
    }
    TAN_ASSERT(rhs->is_lvalue());
    p->set_lvalue(true);
    p->set_type(pcast<PointerType>(rhs_type)->get_pointee());
    break;
  case UnaryOpKind::PLUS:
  case UnaryOpKind::MINUS: /// unary plus/minus
    if (!rhs_type->is_num()) {
      error(ErrorType::TYPE_ERROR, rhs, "Expect a numerical type");
    }
    p->set_type(rhs_type);
    break;
  default:
    TAN_ASSERT(false);
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, Cast) {
  Expr *lhs = p->get_lhs();
  visit(lhs);
  p->set_type(resolve_type(p->get_type(), p));
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, Assignment) {
  Expr *rhs = p->get_rhs();
  visit(rhs);

  auto *lhs = p->get_lhs();
  Type *lhs_type = nullptr;
  if (lhs->get_node_type() == ASTNodeType::VAR_DECL) {
    auto *var_decl = pcast<VarDecl>(lhs);

    // deduce type of variable declaration
    if (!var_decl->get_type()) {
      var_decl->set_type(rhs->get_type());
    }

    visit(lhs);
    lhs_type = var_decl->get_type();
  } else {
    visit(lhs);

    switch (lhs->get_node_type()) {
    case ASTNodeType::ID: {
      auto *id = pcast<Identifier>(lhs);
      if (id->get_id_type() != IdentifierType::ID_VAR_REF) {
        error(ErrorType::TYPE_ERROR, lhs, "Can only assign value to a variable");
      }
      lhs_type = id->get_type();
      break;
    }
    case ASTNodeType::ARG_DECL:
    case ASTNodeType::BOP_OR_UOP:
    case ASTNodeType::UOP:
    case ASTNodeType::BOP:
      lhs_type = pcast<Expr>(lhs)->get_type();
      break;
    default:
      error(ErrorType::TYPE_ERROR, lhs, "Invalid left-hand operand");
    }
  }
  p->set_type(lhs_type);

  rhs = create_implicit_conversion(rhs, lhs_type);
  p->set_rhs(rhs);
  p->set_lvalue(true);
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, FunctionCall) {
  analyze_function_call(p, false); // intrinsic function call is handled elsewhere
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, FunctionDecl) {
  analyze_func_decl_prototype(p);
  analyze_func_body(p);
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, Intrinsic) {
  switch (p->get_intrinsic_type()) {
  case IntrinsicType::LINENO: {
    auto sub = IntegerLiteral::Create(p->src(), _sm->get_line(p->start()), true);
    auto type = PrimitiveType::GetIntegerType(32, true);
    sub->set_type(type);
    p->set_type(type);
    p->set_sub(sub);
    break;
  }
  case IntrinsicType::FILENAME: {
    auto sub = StringLiteral::Create(p->src(), _sm->get_filename());
    auto type = Type::GetStringType();
    sub->set_type(type);
    p->set_type(type);
    p->set_sub(sub);
    break;
  }
  case IntrinsicType::TEST_COMP_ERROR: {
    bool error_caught = false;

    try {
      auto *sub = p->get_sub();
      if (sub) {
        visit(sub);
      } else { // sub is nullptr if it's already checked
        error_caught = true;
      }
    } catch (const CompileException &e) {
      error_caught = true;
      std::cerr << fmt::format("Caught expected compile error: {}\nContinue compilation...\n", e.what());
    }

    if (!error_caught)
      error(ErrorType::TYPE_ERROR, p, "Expect a compile error");
    break;
  }
  case IntrinsicType::NOOP:
    break;
  case IntrinsicType::INVALID:
    TAN_ASSERT(false);
    break;
  default: {
    auto *c = p->get_sub();
    if (c->get_node_type() == ASTNodeType::FUNC_CALL) {
      analyze_intrinsic_func_call(p, pcast<FunctionCall>(c));
      return;
    }

    break;
  }
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, StringLiteral) {
  TAN_ASSERT(!p->get_value().empty());
  p->set_type(Type::GetStringType());
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, CharLiteral) { p->set_type(Type::GetCharType()); }

DEFINE_AST_VISITOR_IMPL(TypeCheck, IntegerLiteral) {
  Type *ty;
  if (p->is_unsigned()) {
    ty = Type::GetIntegerType(32, true);
  } else {
    ty = Type::GetIntegerType(32, false);
  }
  p->set_type(ty);
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, BoolLiteral) { p->set_type(Type::GetBoolType()); }

DEFINE_AST_VISITOR_IMPL(TypeCheck, FloatLiteral) { p->set_type(Type::GetFloatType(32)); }

DEFINE_AST_VISITOR_IMPL(TypeCheck, ArrayLiteral) {
  // TODO IMPORTANT: find the type that all elements can implicitly convert to
  //  for example: [1, 2.2, 3u] has element type float
  auto elements = p->get_elements();
  Type *element_type = nullptr;
  for (auto *e : elements) {
    visit(e);
    if (!element_type) {
      element_type = e->get_type();
    }
    create_implicit_conversion(e, element_type);
  }

  TAN_ASSERT(element_type);
  p->set_type(Type::GetArrayType(element_type, (int)elements.size()));
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, MemberAccess) {
  Expr *lhs = p->get_lhs();
  visit(lhs);

  Expr *rhs = p->get_rhs();

  if (rhs->get_node_type() == ASTNodeType::FUNC_CALL) { /// method call
    p->_access_type = MemberAccess::MemberAccessMemberFunction;
    auto func_call = pcast<FunctionCall>(rhs);
    analyze_member_func_call(p, lhs, func_call);
  } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
    analyze_bracket_access(p, lhs, rhs);
  } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable
    p->_access_type = MemberAccess::MemberAccessMemberVariable;
    analyze_member_access_member_variable(p, lhs, rhs);
  } else {
    error(ErrorType::UNKNOWN_SYMBOL, p, "Invalid right-hand operand");
  }
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, StructDecl) {
  str struct_name = p->get_name();
  auto *ty = pcast<StructType>(p->get_type());
  TAN_ASSERT(ty && ty->is_struct());
  auto members = p->get_member_decls();

  push_scope(p);

  size_t n = members.size();
  for (size_t i = 0; i < n; ++i) {
    Expr *m = members[i];

    if (m->get_node_type() == ASTNodeType::VAR_DECL) { // member variable without initial value
      (*ty)[i] = resolve_type((*ty)[i], m);

    } else if (m->get_node_type() == ASTNodeType::ASSIGN) { // member variable with an initial value
      auto init_val = pcast<Assignment>(m)->get_rhs();
      visit(init_val);

      (*ty)[i] = resolve_type((*ty)[i], m);

      if (!init_val->is_comptime_known()) {
        error(ErrorType::TYPE_ERROR, p, "Initial value of a member variable must be compile-time known");
      }

    } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) { // TODO: member functions
      auto f = pcast<FunctionDecl>(m);
      (*ty)[i] = f->get_type();

    } else {
      error(ErrorType::TYPE_ERROR, p, "Invalid struct member");
    }
  }

  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, Loop) {
  push_scope(p);

  visit(p->get_predicate());
  visit(p->get_body());

  pop_scope();
}

DEFINE_AST_VISITOR_IMPL(TypeCheck, BreakContinue) {
  Loop *loop = search_node_in_parent_scopes<Loop, ASTNodeType::LOOP>();
  if (!loop) {
    error(ErrorType::SEMANTIC_ERROR, p, "Break or continue must be inside a loop");
  }
  p->set_parent_loop(pcast<Loop>(loop));
}

} // namespace tanlang
