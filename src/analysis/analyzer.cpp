#include "analysis/analyzer.h"
#include "ast/ast_base.h"
#include "ast/type.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "ast/context.h"
#include "lexer/token.h"
#include "compiler/compiler.h"
#include "analysis/dependency_graph.h"
#include <iostream>
#include <csetjmp>
#include <set>

namespace tanlang {

using analyze_func_t = void (AnalyzerImpl::*)(ASTBase *);

class AnalyzerImpl final {
public:
  explicit AnalyzerImpl(SourceManager *sm) : _sm(sm) {}

  vector<ASTBase *> sorted_unresolved_symbols() const { return _unresolved_symbols.topological_sort(); }

  void analyze_top_level_declarations(Program *p) {
    _strict = false;
    push_scope(p);

    for (const auto &c : p->get_children()) {
      if (c->get_node_type() == ASTNodeType::IMPORT) {
        analyze_import(c);
      } else if (c->get_node_type() == ASTNodeType::STRUCT_DECL) {
        analyze_struct_decl(c);
      } else if (c->get_node_type() == ASTNodeType::FUNC_DECL) {
        analyze_func_decl_prototype(c);
      }
    }

    pop_scope();
  }

  void analyze(Program *p, const vector<ASTBase *> &sorted_top_level_decls) {
    _strict = true;
    push_scope(p);

    for (auto *c : sorted_top_level_decls) {
      analyze_ast(c);
    }

    std::set<ASTBase *> s(sorted_top_level_decls.begin(), sorted_top_level_decls.end());
    for (auto *c : p->get_children()) {
      if (!s.contains(c))
        analyze_ast(c);
    }

    pop_scope();
  }

private:
  void analyze_ast(ASTBase *p) {
    TAN_ASSERT(p);

    switch (p->get_node_type()) {
    case ASTNodeType::PROGRAM:
    case ASTNodeType::COMPOUND_STATEMENT:
      analyze_compound_stmt(p);
      break;
    case ASTNodeType::RET:
      analyze_ret(p);
      break;
    case ASTNodeType::IF:
      analyze_if(p);
      break;
    case ASTNodeType::IMPORT:
      analyze_import(p);
      break;
    case ASTNodeType::LOOP:
      analyze_loop(p);
      break;
    case ASTNodeType::BREAK:
    case ASTNodeType::CONTINUE:
      analyze_break_or_continue(p);
      break;
      /// expressions
    case ASTNodeType::ASSIGN:
    case ASTNodeType::CAST:
    case ASTNodeType::BOP:
    case ASTNodeType::UOP:
    case ASTNodeType::BOP_OR_UOP:
    case ASTNodeType::ID:
    case ASTNodeType::STRING_LITERAL:
    case ASTNodeType::CHAR_LITERAL:
    case ASTNodeType::BOOL_LITERAL:
    case ASTNodeType::INTEGER_LITERAL:
    case ASTNodeType::FLOAT_LITERAL:
    case ASTNodeType::ARRAY_LITERAL:
    case ASTNodeType::INTRINSIC:
    case ASTNodeType::PARENTHESIS:
    case ASTNodeType::FUNC_CALL:
    case ASTNodeType::FUNC_DECL:
    case ASTNodeType::ARG_DECL:
    case ASTNodeType::VAR_DECL:
    case ASTNodeType::STRUCT_DECL:
      analyze_expr(ast_cast<Expr>(p));
      break;
    default:
      TAN_ASSERT(false);
    }
  }

  /**
   * \brief Check whether it's legal to implicitly convert from type `from` to type `to`
   *        See TYPE_CASTING.md for specifications.
   * \param from Source type.
   * \param to Destination type.
   */
  static bool CanImplicitlyConvert(Type *from, Type *to);

  /**
   * \brief Find out which one of the two input types of a binary operation should operands promote to.
   *        See TYPE_CASTING.md for specifications.
   * \return Guaranteed to be one of `t1` and `t2`, or nullptr if cannot find a legal promotion.
   */
  static Type *ImplicitTypePromote(Type *t1, Type *t2);

  SourceManager *_sm = nullptr;
  bool _strict = false;
  vector<ASTBase *> _scopes{};

  /**
   * \brief Store unresolved symbols during non-strict parsing
   */
  DependencyGraph _unresolved_symbols{};

  void push_scope(ASTBase *scope) { _scopes.push_back(scope); }
  void pop_scope() {
    TAN_ASSERT(!_scopes.empty());
    _scopes.pop_back();
  }
  Context *top_ctx() {
    TAN_ASSERT(!_scopes.empty());
    return _scopes.back()->ctx();
  }

private:
  [[noreturn]] void error(ASTBase *p, const str &message) {
    Error err(_sm->get_filename(), _sm->get_token(p->loc()), message);
    err.raise();
  }

  Cast *create_implicit_conversion(Expr *from, Type *to) {
    if (!CanImplicitlyConvert(from->get_type(), to)) {
      error(from, fmt::format("Cannot implicitly convert type {} to {}", from->get_type()->get_typename(),
                              to->get_typename()));
    }

    auto *cast = Cast::Create(from->loc());
    cast->set_lhs(from);
    cast->set_type(to);
    return cast;
  }

  /**
   * \brief Find the type that operands of a BOP should promote to, and add a Cast node to the AST.
   *        Raise an error if can't find a valid type promotion.
   * \note This could modify the lhs or rhs of `bop`, make sure to update the references to any of them after calling.
   * \return The promoted type.
   */
  Type *auto_promote_bop_operand_types(BinaryOperator *bop) {
    auto *lhs = bop->get_lhs();
    auto *rhs = bop->get_rhs();
    auto *lhs_type = lhs->get_type();
    auto *rhs_type = rhs->get_type();

    auto *promoted_type = ImplicitTypePromote(lhs_type, rhs_type);
    if (!promoted_type) {
      error(bop, fmt::format("Cannot find a valid type promotion between {} and {}", lhs_type->get_typename(),
                             rhs_type->get_typename()));
    }

    TAN_ASSERT(promoted_type == lhs_type || promoted_type == rhs_type);
    if (promoted_type != lhs_type) {
      auto *cast = Cast::Create(bop->loc());
      cast->set_lhs(lhs);
      cast->set_type(promoted_type);
      bop->set_lhs(cast);
    } else {
      auto *cast = Cast::Create(bop->loc());
      cast->set_lhs(rhs);
      cast->set_type(promoted_type);
      bop->set_rhs(cast);
    }

    return promoted_type;
  }

  FunctionDecl *search_function_callee(FunctionCall *p) {
    const str &name = p->get_name();
    const vector<Expr *> &args = p->_args;

    /// gather all candidates from this and parent scopes
    vector<FunctionDecl *> func_candidates = _scopes.front()->ctx()->get_functions(name);

    /// find a valid function overload to call
    FunctionDecl *ret = nullptr;
    for (const auto &f : func_candidates) {
      size_t n = f->get_n_args();
      if (n != args.size()) {
        continue;
      }

      auto *func_type = (FunctionType *)f->get_type();

      /// check if argument types match (return type not checked)
      /// allow implicit cast from actual arguments to expected arguments
      bool good = true;
      int cost = 0; /// number of implicit type conversion of arguments needed
      for (size_t i = 0; i < n; ++i) {
        auto *actual_type = args[i]->get_type();
        auto *expected_type = func_type->get_arg_types()[i];

        if (actual_type != expected_type) {
          ++cost;

          if (!CanImplicitlyConvert(actual_type, expected_type)) {
            good = false;
            break;
          }
        }
      }

      /// remember valid candidate(s) and check for ambiguity
      if (good) {
        /// if there is an exact match, use it
        if (cost == 0) {
          ret = f;
          break;
        }

        if (ret) {
          // TODO: print all valid candidates
          error(p, "Ambiguous function call: " + name);
        }
        ret = f;
      }
    }

    if (!ret) {
      Error err(_sm->get_filename(), _sm->get_token(p->loc()), "Unknown function call: " + name);
      err.raise();
    }
    return ret;
  }

  Decl *search_decl_in_scopes(const str &name) {
    int n = (int)_scopes.size();
    TAN_ASSERT(n);
    Decl *ret = nullptr;
    for (int i = n - 1; i >= 0; --i) {
      Context *c = _scopes[(size_t)i]->ctx();
      ret = c->get_decl(name);
      if (ret)
        return ret;
    }

    return ret;
  }

  Loop *search_loop_in_parent_scopes() {
    int n = (int)_scopes.size();
    TAN_ASSERT(n);
    for (int i = n - 1; i >= 0; --i) {
      auto *node = _scopes[(size_t)i];
      if (node->get_node_type() == ASTNodeType::LOOP) {
        return ast_cast<Loop>(node);
      }
    }

    return nullptr;
  }

  void analyze_expr(Expr *p) { (this->*EXPRESSION_ANALYZER_TABLE[p->get_node_type()])(p); }

  /**
   * \brief Resolve type reference.
   *        In non-strict mode, this adds a dependency from \p node to the referred declaration D
   *        if D doesn't have a resolved type yet.
   *        In strict mode, an error is raised.
   * \return The referred type if successfully resolved. Return \p p as is if failed.
   */
  Type *resolve_type_ref(Type *p, SrcLoc loc, ASTBase *node) {
    TAN_ASSERT(p->is_ref());
    Type *ret = p;

    const str &referred_name = p->get_typename();
    auto *decl = search_decl_in_scopes(referred_name);
    if (decl && decl->is_type_decl()) {
      if (!decl->get_type() || !decl->get_type()->is_resolved()) {
        if (_strict) {
          Error err(_sm->get_filename(), _sm->get_token(loc), fmt::format("Unknown type {}", referred_name));
          err.raise();
        } else {
          _unresolved_symbols.add_dependency(decl, node);
        }
      } else {
        ret = decl->get_type();
      }
    } else { // no matter we're in strict mode or not,
             // the typename should've been registered after parsing and importing.
      Error err(_sm->get_filename(), _sm->get_token(loc), fmt::format("Unknown type {}", referred_name));
      err.raise();
    }

    return ret;
  }

  /**
   * \brief Resolve a type. If \p is a type reference, we find out the type associated with the typename.
   * \note Returned pointer can be different from \p p.
   */
  Type *resolve_type(Type *p, SrcLoc loc, ASTBase *node) {
    TAN_ASSERT(p);
    TAN_ASSERT(node);

    Type *ret = p;
    if (p->is_ref()) {
      ret = resolve_type_ref(p, loc, node);
    } else if (p->is_pointer()) {
      auto *pointee = ((PointerType *)p)->get_pointee();

      TAN_ASSERT(pointee);
      if (pointee->is_ref()) {
        pointee = resolve_type_ref(pointee, loc, node);
        if (pointee->is_resolved()) {
          ret = Type::GetPointerType(pointee);
        }
      }
    }

    TAN_ASSERT(ret);
    return ret;
  }

  void analyze_id(ASTBase *_p) {
    auto p = ast_cast<Identifier>(_p);
    auto *referred = search_decl_in_scopes(p->get_name());
    if (referred) {
      if (referred->is_type_decl()) { /// refers to a type
        auto *ty = resolve_type_ref(referred->get_type(), p->loc(), p);
        p->set_type_ref(ty);
      } else { /// refers to a variable
        p->set_var_ref(VarRef::Create(p->loc(), p->get_name(), referred));
        p->set_type(resolve_type(referred->get_type(), p->loc(), p));
      }
    } else {
      error(p, "Unknown identifier");
    }
  }

  void analyze_parenthesis(ASTBase *_p) {
    auto p = ast_cast<Parenthesis>(_p);
    analyze_ast(p->get_sub());
    p->set_type(p->get_sub()->get_type());
  }

  // TODO: decouple if branch and else clause because they each have a different context/scope
  void analyze_if(ASTBase *_p) {
    auto p = ast_cast<If>(_p);

    size_t n = p->get_num_branches();
    for (size_t i = 0; i < n; ++i) {
      auto *cond = p->get_predicate(i);
      if (cond) { /// can be nullptr, meaning an "else" branch
        analyze_ast(cond);
        p->set_predicate(i, create_implicit_conversion(cond, Type::GetBoolType()));
      }

      analyze_ast(p->get_branch(i));
    }
  }

  void analyze_var_decl(ASTBase *_p) {
    auto p = ast_cast<VarDecl>(_p);

    // NOTE: analyze_assignment must've set the type by now

    Type *ty = p->get_type();
    p->set_type(resolve_type(ty, p->loc(), p));

    top_ctx()->set_decl(p->get_name(), p);
  }

  void analyze_arg_decl(ASTBase *_p) {
    auto p = ast_cast<ArgDecl>(_p);
    p->set_type(resolve_type(p->get_type(), p->loc(), p));
    top_ctx()->set_decl(p->get_name(), p);
  }

  void analyze_ret(ASTBase *_p) {
    // TODO: check if return type is the same as the function return type
    auto p = ast_cast<Return>(_p);
    auto *rhs = p->get_rhs();
    if (rhs) {
      analyze_ast(rhs);
    }
  }

  void analyze_compound_stmt(ASTBase *_p) {
    auto p = ast_cast<CompoundStmt>(_p);
    push_scope(p);

    for (const auto &c : p->get_children()) {
      analyze_ast(c);
    }

    pop_scope();
  }

  void analyze_bop_or_uop(ASTBase *_p) {
    auto p = ast_cast<BinaryOrUnary>(_p);
    analyze_ast(p->get_expr_ptr());
  }

  void analyze_bop(ASTBase *_p) {
    auto p = ast_cast<BinaryOperator>(_p);
    Expr *lhs = p->get_lhs();
    Expr *rhs = p->get_rhs();

    if (p->get_op() == BinaryOpKind::MEMBER_ACCESS) {
      analyze_member_access(ast_cast<MemberAccess>(p));
      return;
    }

    analyze_ast(lhs);
    analyze_ast(rhs);

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

  void analyze_uop(ASTBase *_p) {
    auto *p = ast_cast<UnaryOperator>(_p);
    auto *rhs = p->get_rhs();
    analyze_ast(rhs);

    auto *rhs_type = rhs->get_type();
    switch (p->get_op()) {
    case UnaryOpKind::LNOT:
      rhs = create_implicit_conversion(rhs, Type::GetBoolType());
      p->set_rhs(rhs);
      p->set_type(PrimitiveType::GetBoolType());
      break;
    case UnaryOpKind::BNOT:
      if (!rhs_type->is_int()) {
        error(rhs, "Expect an integer type");
      }
      p->set_type(rhs_type);
      break;
    case UnaryOpKind::ADDRESS_OF:
      p->set_type(Type::GetPointerType(rhs_type));
      break;
    case UnaryOpKind::PTR_DEREF:
      if (!rhs_type->is_pointer()) {
        error(rhs, "Expect a pointer type");
      }
      TAN_ASSERT(rhs->is_lvalue());
      p->set_lvalue(true);
      p->set_type(((PointerType *)rhs_type)->get_pointee());
      break;
    case UnaryOpKind::PLUS:
    case UnaryOpKind::MINUS: /// unary plus/minus
      if (!(rhs_type->is_int() || rhs_type->is_float())) {
        error(rhs, "Expect an numerical type");
      }
      p->set_type(rhs_type);
      break;
    default:
      TAN_ASSERT(false);
    }
  }

  void analyze_cast(ASTBase *_p) {
    auto *p = ast_cast<Cast>(_p);
    Expr *lhs = p->get_lhs();
    analyze_ast(lhs);
    p->set_type(resolve_type(p->get_type(), p->loc(), p));
  }

  void analyze_assignment(ASTBase *_p) {
    auto *p = ast_cast<Assignment>(_p);

    Expr *rhs = p->get_rhs();
    analyze_ast(rhs);

    auto *lhs = p->get_lhs();
    Type *lhs_type = nullptr;
    if (lhs->get_node_type() == ASTNodeType::VAR_DECL) {
      /// special case for variable declaration because we allow type inference
      lhs_type = rhs->get_type();
      ast_cast<VarDecl>(lhs)->set_type(lhs_type);
      analyze_ast(lhs);
    } else {
      analyze_ast(lhs);

      switch (lhs->get_node_type()) {
      case ASTNodeType::ID: {
        auto *id = ast_cast<Identifier>(lhs);
        if (id->get_id_type() != IdentifierType::ID_VAR_REF) {
          error(lhs, "Can only assign value to a variable");
        }
        lhs_type = id->get_type();
        break;
      }
      case ASTNodeType::ARG_DECL:
      case ASTNodeType::BOP_OR_UOP:
      case ASTNodeType::UOP:
      case ASTNodeType::BOP:
        lhs_type = ast_cast<Expr>(lhs)->get_type();
        break;
      default:
        error(lhs, "Invalid left-hand operand");
      }
    }
    p->set_type(lhs_type);

    rhs = create_implicit_conversion(rhs, lhs_type);
    p->set_rhs(rhs);
    p->set_lvalue(true);
  }

  void analyze_func_call(ASTBase *_p) {
    auto p = ast_cast<FunctionCall>(_p);

    for (const auto &a : p->_args) {
      analyze_ast(a);
    }

    FunctionDecl *callee = search_function_callee(p);
    p->_callee = callee;
    auto *func_type = (FunctionType *)callee->get_type();
    p->set_type(func_type->get_return_type());
  }

  void analyze_func_decl(ASTBase *p) {
    analyze_func_decl_prototype(p);
    analyze_func_body(p);
  }

  void analyze_func_decl_prototype(ASTBase *_p) {
    auto *p = ast_cast<FunctionDecl>(_p);

    _scopes.front()->ctx()->add_function_decl(p);

    push_scope(p);

    /// update return type
    auto *func_type = (FunctionType *)p->get_type();
    auto *ret_type = resolve_type(func_type->get_return_type(), p->loc(), p);
    if (!ret_type->is_resolved()) {
      TAN_ASSERT(!_strict);
    }
    func_type->set_return_type(ret_type);

    /// type_check_ast args
    size_t n = p->get_n_args();
    const auto &arg_decls = p->get_arg_decls();
    vector<Type *> arg_types(n, nullptr);
    for (size_t i = 0; i < n; ++i) {
      analyze_ast(arg_decls[i]); /// args will be added to the scope here
      arg_types[i] = arg_decls[i]->get_type();

      if (!arg_types[i]->is_resolved()) {
        TAN_ASSERT(!_strict);
        _unresolved_symbols.add_dependency(arg_decls[i], p);
      }
    }
    func_type->set_arg_types(arg_types); /// update arg types

    pop_scope();
  }

  void analyze_func_body(ASTBase *_p) {
    auto *p = ast_cast<FunctionDecl>(_p);

    push_scope(p);

    if (!p->is_external()) {
      analyze_ast(p->get_body());
    }

    pop_scope();
  }

  // TODO: analyze top-level declarations of the imported files because they might contains unresolved symbols
  // TODO: check recursive import
  void analyze_import(ASTBase *_p) {
    auto p = ast_cast<Import>(_p);

    str file = p->get_filename();
    auto imported = Compiler::resolve_import(_sm->get_filename(), file);
    if (imported.empty()) {
      error(p, "Cannot import: " + file);
    }

    auto *compiler = new Compiler(imported[0]);
    compiler->parse(); // only need to parse the file now
    Context *imported_ctx = compiler->get_root_ast()->ctx();

    // import functions
    vector<FunctionDecl *> funcs = imported_ctx->get_functions();
    vector<FunctionDecl *> pub_funcs{};
    for (auto *f : funcs) {
      if (f->is_public() || f->is_external()) {
        pub_funcs.push_back(f);
        top_ctx()->add_function_decl(f);
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

  void analyze_intrinsic_func_call(Intrinsic *p, FunctionCall *func_call) {
    auto *void_type = Type::GetVoidType();
    switch (p->get_intrinsic_type()) {
    case IntrinsicType::STACK_TRACE: {
      func_call->set_name(Intrinsic::STACK_TRACE_FUNCTION_REAL_NAME);
      analyze_ast(func_call);
      p->set_type(void_type);
      break;
    }
    case IntrinsicType::ABORT:
    case IntrinsicType::NOOP:
      analyze_ast(func_call);
      p->set_type(void_type);
      break;
    case IntrinsicType::GET_DECL: {
      if (func_call->get_n_args() != 1) {
        error(func_call, "Expect the number of args to be 1");
      }
      auto *target = func_call->get_arg(0);
      auto *source_str = Literal::CreateStringLiteral(p->loc(), _sm->get_source_code(target->loc()));

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
        error(p, "Invalid call to compprint, one argument with type 'str' required");
      }

      str msg = ast_cast<StringLiteral>(args[0])->get_value();
      std::cout << fmt::format("Message ({}): {}\n", _sm->get_src_location_str(p->loc()), msg);
      break;
    }
    default:
      TAN_ASSERT(false);
    }
  }

  /// search for the intrinsic type
  inline void find_and_assign_intrinsic_type(Intrinsic *p, const str &name) {
    auto q = Intrinsic::intrinsics.find(name);
    if (q == Intrinsic::intrinsics.end()) {
      error(p, fmt::format("Unknown intrinsic {}", name));
    }
    p->set_intrinsic_type(q->second);
  }

  void analyze_intrinsic(ASTBase *_p) {
    auto p = ast_cast<Intrinsic>(_p);
    auto c = p->get_sub();

    /// name
    str name;
    switch (c->get_node_type()) {
    case ASTNodeType::FUNC_CALL: {
      auto *func_call = ast_cast<FunctionCall>(c);
      name = func_call->get_name();
      find_and_assign_intrinsic_type(p, name);
      analyze_intrinsic_func_call(p, func_call);
      return;
    }
    case ASTNodeType::ID:
      name = ast_cast<Identifier>(c)->get_name();
      break;
    default:
      name = p->get_name();
      break;
    }
    TAN_ASSERT(!name.empty());
    find_and_assign_intrinsic_type(p, name);

    switch (p->get_intrinsic_type()) {
    case IntrinsicType::LINENO: {
      auto sub = IntegerLiteral::Create(p->loc(), _sm->get_line(p->loc()), true);
      auto type = PrimitiveType::GetIntegerType(32, true);
      sub->set_type(type);
      p->set_type(type);
      p->set_sub(sub);
      break;
    }
    case IntrinsicType::FILENAME: {
      auto sub = StringLiteral::Create(p->loc(), _sm->get_filename());
      auto type = Type::GetStringType();
      sub->set_type(type);
      p->set_type(type);
      p->set_sub(sub);
      break;
    }
    case IntrinsicType::TEST_COMP_ERROR: {
      // FIXME: avoid setjmp and longjmp
      bool error_catched = false;
      std::jmp_buf buf;
      if (setjmp(buf) > 0) {
        error_catched = true;
      } else {
        auto error_catcher = ErrorCatcher((const ErrorCatcher::callback_t &)[&](str) { longjmp(buf, 1); });
        Error::CatchErrors(&error_catcher);
        analyze_ast(p->get_sub());
      }

      Error::ResetErrorCatcher();
      if (!error_catched) {
        error(p, "Expect a compile error");
      }
      break;
    }
    default:
      TAN_ASSERT(false);
    }
  }

  void analyze_string_literal(ASTBase *_p) {
    auto p = ast_cast<StringLiteral>(_p);
    p->set_value(_sm->get_token_str(p->loc()));
    p->set_type(Type::GetStringType());
  }

  void analyze_char_literal(ASTBase *_p) {
    auto p = ast_cast<CharLiteral>(_p);
    p->set_type(Type::GetCharType());
    p->set_value(static_cast<uint8_t>(_sm->get_token_str(p->loc())[0]));
  }

  void analyze_integer_literal(ASTBase *_p) {
    auto p = ast_cast<IntegerLiteral>(_p);

    Type *ty;
    if (_sm->get_token(p->loc())->is_unsigned()) {
      ty = Type::GetIntegerType(32, true);
    } else {
      ty = Type::GetIntegerType(32, false);
    }
    p->set_type(ty);
  }

  void analyze_bool_literal(ASTBase *_p) {
    auto p = ast_cast<BoolLiteral>(_p);
    p->set_type(Type::GetBoolType());
  }

  void analyze_float_literal(ASTBase *_p) {
    auto p = ast_cast<FloatLiteral>(_p);
    p->set_type(Type::GetFloatType(32));
  }

  void analyze_array_literal(ASTBase *_p) {
    auto p = ast_cast<ArrayLiteral>(_p);

    // TODO IMPORTANT: find the type that all elements can implicitly convert to
    //  for example: [1, 2.2, 3u] has element type float
    auto elements = p->get_elements();
    Type *element_type = nullptr;
    for (auto *e : elements) {
      analyze_ast(e);
      if (!element_type) {
        element_type = e->get_type();
      }
      create_implicit_conversion(e, element_type);
    }

    TAN_ASSERT(element_type);
    p->set_type(Type::GetArrayType(element_type, (int)elements.size()));
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_member_func_call(MemberAccess *p, Expr *lhs, FunctionCall *rhs) {
    if (!lhs->is_lvalue() && !lhs->get_type()->is_pointer()) {
      error(p, "Invalid member function call");
    }

    // insert the address of the struct instance as the first parameter
    if (lhs->is_lvalue() && !lhs->get_type()->is_pointer()) {
      Expr *tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs->loc(), lhs);
      analyze_ast(tmp);
      rhs->_args.insert(rhs->_args.begin(), tmp);
    } else {
      rhs->_args.insert(rhs->_args.begin(), lhs);
    }

    analyze_ast(rhs);
    p->set_type(rhs->get_type());
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_bracket_access(MemberAccess *p, Expr *lhs, Expr *rhs) {
    analyze_ast(rhs);

    if (!lhs->is_lvalue()) {
      error(p, "Expect lhs to be an lvalue");
    }

    auto *lhs_type = lhs->get_type();
    if (!(lhs_type->is_pointer() || lhs_type->is_array() || lhs_type->is_string())) {
      error(p, "Expect a type that supports bracket access");
    }
    if (!rhs->get_type()->is_int()) {
      error(rhs, "Expect an integer");
    }

    Type *sub_type = nullptr;
    if (lhs_type->is_pointer()) {
      sub_type = ((PointerType *)lhs_type)->get_pointee();
    } else if (lhs_type->is_array()) {
      auto *array_type = (ArrayType *)lhs_type;
      sub_type = array_type->get_element_type();
      /// check if array index is out-of-bound
      if (rhs->get_node_type() == ASTNodeType::INTEGER_LITERAL) {
        uint64_t size = ast_cast<IntegerLiteral>(rhs)->get_value();
        if (lhs->get_type()->is_array() && (int)size >= array_type->get_size()) {
          error(p, fmt::format("Index {} out of bound, the array size is {}", std::to_string(size),
                               std::to_string(array_type->get_size())));
        }
      }
    } else if (lhs_type->is_string()) {
      sub_type = Type::GetCharType();
    }

    p->set_type(sub_type);
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_member_access_member_variable(MemberAccess *p, Expr *lhs, Expr *rhs) {
    str m_name = ast_cast<Identifier>(rhs)->get_name();
    Type *struct_ty = nullptr;
    /// auto dereference pointers
    if (lhs->get_type()->is_pointer()) {
      struct_ty = ((PointerType *)lhs->get_type())->get_pointee();
    } else {
      struct_ty = lhs->get_type();
    }

    struct_ty = resolve_type(struct_ty, lhs->loc(), p);
    if (!struct_ty->is_struct()) {
      error(lhs, "Expect a struct type");
    }

    auto *struct_decl = ast_cast<StructDecl>(search_decl_in_scopes(struct_ty->get_typename()));
    p->_access_idx = struct_decl->get_struct_member_index(m_name);
    auto *ty = struct_decl->get_struct_member_ty(p->_access_idx);
    p->set_type(resolve_type(ty, p->loc(), p));
  }

  void analyze_member_access(MemberAccess *p) {
    Expr *lhs = p->get_lhs();
    analyze_ast(lhs);

    Expr *rhs = p->get_rhs();

    if (rhs->get_node_type() == ASTNodeType::FUNC_CALL) { /// method call
      p->_access_type = MemberAccess::MemberAccessMemberFunction;
      auto func_call = ast_cast<FunctionCall>(rhs);
      analyze_member_func_call(p, lhs, func_call);
    } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
      analyze_bracket_access(p, lhs, rhs);
    } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable
      p->_access_type = MemberAccess::MemberAccessMemberVariable;
      analyze_member_access_member_variable(p, lhs, rhs);
    } else {
      error(p, "Invalid right-hand operand");
    }
  }

  void analyze_struct_decl(ASTBase *_p) {
    auto p = ast_cast<StructDecl>(_p);

    str struct_name = p->get_name();

    /// check if struct name is in conflicts of variable/function names
    if (!p->is_forward_decl()) {
      auto *root_ctx = _scopes.front()->ctx();
      auto *prev_decl = root_ctx->get_decl(struct_name);
      if (prev_decl && prev_decl != p) {
        if (!(prev_decl->get_node_type() == ASTNodeType::STRUCT_DECL &&
              ast_cast<StructDecl>(prev_decl)->is_forward_decl()))
          error(p, "Cannot redeclare type as a struct");
      }
      // overwrite the value set during parsing (e.g. forward decl)
      root_ctx->set_decl(struct_name, p);
    }

    push_scope(p);

    /// resolve member names and types
    auto member_decls = p->get_member_decls();
    size_t n = member_decls.size();
    // create the type first and modify the member types on the fly to support recursive type reference
    TAN_ASSERT(!p->get_type() || p->get_type()->is_struct());
    auto *ty = (StructType *)p->get_type();
    if (!ty) {
      ty = Type::GetStructType(struct_name, vector<Type *>(n, nullptr));
      p->set_type(ty);
    }
    for (size_t i = 0; i < n; ++i) {
      Expr *m = member_decls[i];
      analyze_ast(m); // TODO IMPORTANT: don't save member variable declarations to Context

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
      if (!m->get_type()->is_resolved() && !m->get_type()->is_pointer()) {
        TAN_ASSERT(!_strict);
        _unresolved_symbols.add_dependency(m, p);
      }

      if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
        /// fill members
        str name = ast_cast<VarDecl>(m)->get_name();
        p->set_member_index(name, i);
        (*ty)[i] = m->get_type();
      } else if (m->get_node_type() == ASTNodeType::ASSIGN) { /// member variable with an initial value
        auto bm = ast_cast<Assignment>(m);
        auto init_val = bm->get_rhs();

        if (bm->get_lhs()->get_node_type() != ASTNodeType::VAR_DECL) {
          error(bm, "Expect a member variable declaration");
        }
        auto decl = ast_cast<VarDecl>(bm->get_lhs());

        /// fill members
        (*ty)[i] = decl->get_type();
        p->set_member_index(decl->get_name(), i);

        /// initial values
        if (!init_val->is_comptime_known()) {
          error(p, "Initial value of a member variable must be compile-time known");
        }
        // TODO: auto *ctr = cast_ptr<StructConstructor>(ty->get_constructor());
        //   ctr->get_member_constructors().push_back(BasicConstructor::Create(ast_cast<CompTimeExpr>(init_val)));
      } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) { /// member functions
        auto f = ast_cast<FunctionDecl>(m);

        /// fill members
        (*ty)[i] = f->get_type();
        p->set_member_index(f->get_name(), i);
      } else {
        error(p, "Invalid struct member");
      }
    }

    pop_scope();
  }

  void analyze_loop(ASTBase *_p) {
    auto *p = ast_cast<Loop>(_p);
    push_scope(p);

    analyze_ast(p->get_predicate());
    analyze_ast(p->get_body());

    pop_scope();
  }

  void analyze_break_or_continue(ASTBase *_p) {
    auto *p = ast_cast<BreakContinue>(_p);

    Loop *loop = search_loop_in_parent_scopes();
    if (!loop) {
      error(p, "Break or continue must be inside a loop");
    }
    p->set_parent_loop(ast_cast<Loop>(loop));
  }

private:
  static inline umap<ASTNodeType, analyze_func_t> EXPRESSION_ANALYZER_TABLE{
      {ASTNodeType::ASSIGN,          &AnalyzerImpl::analyze_assignment     },
      {ASTNodeType::CAST,            &AnalyzerImpl::analyze_cast           },
      {ASTNodeType::BOP,             &AnalyzerImpl::analyze_bop            },
      {ASTNodeType::UOP,             &AnalyzerImpl::analyze_uop            },
      {ASTNodeType::ID,              &AnalyzerImpl::analyze_id             },
      {ASTNodeType::STRING_LITERAL,  &AnalyzerImpl::analyze_string_literal },
      {ASTNodeType::CHAR_LITERAL,    &AnalyzerImpl::analyze_char_literal   },
      {ASTNodeType::BOOL_LITERAL,    &AnalyzerImpl::analyze_bool_literal   },
      {ASTNodeType::INTEGER_LITERAL, &AnalyzerImpl::analyze_integer_literal},
      {ASTNodeType::FLOAT_LITERAL,   &AnalyzerImpl::analyze_float_literal  },
      {ASTNodeType::ARRAY_LITERAL,   &AnalyzerImpl::analyze_array_literal  },
      {ASTNodeType::BOP_OR_UOP,      &AnalyzerImpl::analyze_bop_or_uop     },
      {ASTNodeType::INTRINSIC,       &AnalyzerImpl::analyze_intrinsic      },
      {ASTNodeType::PARENTHESIS,     &AnalyzerImpl::analyze_parenthesis    },
      {ASTNodeType::FUNC_CALL,       &AnalyzerImpl::analyze_func_call      },
      {ASTNodeType::FUNC_DECL,       &AnalyzerImpl::analyze_func_decl      },
      {ASTNodeType::ARG_DECL,        &AnalyzerImpl::analyze_arg_decl       },
      {ASTNodeType::VAR_DECL,        &AnalyzerImpl::analyze_var_decl       },
      {ASTNodeType::STRUCT_DECL,     &AnalyzerImpl::analyze_struct_decl    },
  };
};

void Analyzer::analyze(Program *p, const vector<ASTBase *> &sorted_top_level_decls) {
  _analyzer_impl->analyze(p, sorted_top_level_decls);
}

Analyzer::Analyzer(SourceManager *sm) { _analyzer_impl = new AnalyzerImpl(sm); }

Analyzer::~Analyzer() { delete _analyzer_impl; }

void Analyzer::analyze_top_level_declarations(Program *p) { _analyzer_impl->analyze_top_level_declarations(p); }

vector<ASTBase *> Analyzer::sorted_unresolved_symbols() const { return _analyzer_impl->sorted_unresolved_symbols(); }

#include "implicit_cast.hpp"

} // namespace tanlang
