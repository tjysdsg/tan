#include "analyzer.h"
#include "base.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_builder.h"
#include "src/ast/constructor.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/ast/intrinsic.h"
#include "src/analysis/type_system.h"
#include "token.h"
#include "src/ast/ast_context.h"
#include "compiler.h"
#include "src/common.h"
#include <iostream>
#include <fmt/core.h>

namespace tanlang {

class AnalyzerImpl {
public:
  AnalyzerImpl(ASTContext *cs) : _ctx(cs), _sm(cs->get_source_manager()) {}

  void analyze(ASTBase *p) {
    TAN_ASSERT(p);

    switch (p->get_node_type()) {
      case ASTNodeType::PROGRAM:
      case ASTNodeType::STATEMENT:
        analyze_stmt(p);
        break;
      case ASTNodeType::ASSIGN:
        analyze_assignment(ast_must_cast<Assignment>(p));
        break;
      case ASTNodeType::CAST:
        analyze_cast(ast_must_cast<Cast>(p));
        break;
      case ASTNodeType::BOP:
        analyze_bop(p);
        break;
      case ASTNodeType::UOP:
        analyze_uop(p);
        break;
      case ASTNodeType::RET:
        analyze_ret(p);
        break;
      case ASTNodeType::ID:
        analyze_id(p);
        break;
      case ASTNodeType::STRING_LITERAL:
        analyze_string_literal(p);
        break;
      case ASTNodeType::CHAR_LITERAL:
        analyze_char_literal(p);
        break;
      case ASTNodeType::BOOL_LITERAL:
        analyze_bool_literal(p);
        break;
      case ASTNodeType::INTEGER_LITERAL:
        analyze_integer_literal(p);
        break;
      case ASTNodeType::FLOAT_LITERAL:
        analyze_float_literal(p);
        break;
      case ASTNodeType::ARRAY_LITERAL:
        analyze_array_literal(p);
        break;
      case ASTNodeType::IF:
        analyze_if(p);
        break;
      case ASTNodeType::INTRINSIC:
        analyze_intrinsic(p);
        break;
      case ASTNodeType::IMPORT:
        analyze_import(p);
        break;
      case ASTNodeType::PARENTHESIS:
        analyze_parenthesis(p);
        break;
      case ASTNodeType::FUNC_CALL:
        analyze_func_call(p);
        break;
      case ASTNodeType::TY:
        resolve_ty(ast_must_cast<ASTType>(p));
        break;
      case ASTNodeType::ENUM_DECL:
        analyze_enum_decl(p);
        break;
      case ASTNodeType::FUNC_DECL:
        analyze_func_decl(p);
        break;
      case ASTNodeType::ARG_DECL:
        analyze_arg_decl(p);
        break;
      case ASTNodeType::VAR_DECL:
        analyze_var_decl(p);
        break;
      case ASTNodeType::STRUCT_DECL:
        analyze_struct_decl(p);
        break;
      case ASTNodeType::BOP_OR_UOP:
        analyze_bop_or_uop(p);
        break;
      case ASTNodeType::LOOP:
        analyze_loop(p);
        break;
      case ASTNodeType::BREAK:
      case ASTNodeType::CONTINUE:
        analyze_break_or_continue(p);
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  }

private:
  ASTContext *_ctx = nullptr;
  SourceManager *_sm = nullptr;

private:
  ASTType *copy_ty(ASTType *p) const { return new ASTType(*p); }

  void resolve_ty(ASTType *p) const {
    TypeSystem::ResolveTy(_ctx, p);
    TypeSystem::SetDefaultConstructor(_ctx, p);
  }

  [[noreturn]] void report_error(ASTBase *p, const str &message) {
    tanlang::report_error(_ctx->_filename, _sm->get_token(p->get_loc()), message);
  }

  void analyze_id(ASTBase *_p) {
    auto p = ast_must_cast<Identifier>(_p);
    auto referred = _ctx->get(p->get_name());
    if (!referred) {
      report_error(p, "Unknown identifier");
    }

    auto declared = ast_cast<Decl>(referred);
    if (!declared) {
      report_error(p, "Invalid identifier");
    }

    p->_referred = declared;
    auto ty = copy_ty(declared->get_type());
    ty->set_is_lvalue(true);
    p->set_type(ty);
  }

  void analyze_parenthesis(ASTBase *_p) {
    auto p = ast_must_cast<Parenthesis>(_p);

    analyze(p->get_sub());

    p->set_type(copy_ty(p->get_sub()->get_type()));
  }

  void analyze_if(ASTBase *_p) {
    auto p = ast_must_cast<If>(_p);

    size_t n = p->get_num_branches();
    for (size_t i = 0; i < n; ++i) {
      auto cond = p->get_predicate(i);
      if (cond) { /// can be nullptr, meaning an "else" branch
        analyze(cond);
        if (0 != TypeSystem::CanImplicitCast(_ctx,
            ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::BOOL),
            cond->get_type())) {
          report_error(p, "Cannot convert expression to bool");
        }
      }

      analyze(p->get_branch(i));
    }
  }

  void analyze_var_decl(ASTBase *_p) {
    auto p = ast_must_cast<VarDecl>(_p);
    ASTType *ty = p->get_type();

    if (ty) { /// analyze type if specified
      ty->set_is_lvalue(true);
      analyze(ty);
    }

    _ctx->add(p->get_name(), p);
  }

  void analyze_arg_decl(ASTBase *_p) {
    auto p = ast_must_cast<ArgDecl>(_p);
    ASTType *ty = p->get_type();
    ty->set_is_lvalue(true);
    analyze(ty);
    _ctx->add(p->get_name(), p);
  }

  void analyze_ret(ASTBase *_p) {
    // TODO: check if return type can be implicitly cast to function return type
    auto p = ast_must_cast<Return>(_p);
    analyze(p->get_rhs());
  }

  void analyze_stmt(ASTBase *_p) {
    auto p = ast_must_cast<CompoundStmt>(_p);
    for (const auto &c: p->get_children()) {
      analyze(c);
    }
  }

  void analyze_bop_or_uop(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOrUnary>(_p);
    analyze(p->get_generic_ptr());
  }

  void analyze_bop(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOperator>(_p);
    Expr *lhs = p->get_lhs();
    Expr *rhs = p->get_rhs();

    /// NOTE: do not analyze lhs and rhs just yet, because analyze_assignment
    /// and analyze_member_access have their own ways of analyzing

    switch (p->get_op()) {
      case BinaryOpKind::SUM:
      case BinaryOpKind::SUBTRACT:
      case BinaryOpKind::MULTIPLY:
      case BinaryOpKind::DIVIDE:
      case BinaryOpKind::MOD: {
        analyze(lhs);
        analyze(rhs);

        int i = TypeSystem::CanImplicitCast(_ctx, lhs->get_type(), rhs->get_type());
        if (i == -1) {
          report_error(p, "Cannot perform implicit type conversion");
        }

        size_t dominant_idx = static_cast<size_t>(i);
        p->set_dominant_idx(dominant_idx);
        ASTType *ty = copy_ty(dominant_idx == 0 ? lhs->get_type() : rhs->get_type());
        ty->set_is_lvalue(false);
        p->set_type(ty);
        break;
      }
      case BinaryOpKind::BAND:
      case BinaryOpKind::LAND:
      case BinaryOpKind::BOR:
      case BinaryOpKind::LOR:
      case BinaryOpKind::XOR:
        // TODO: implement the type checking of the above operators
        TAN_ASSERT(false);
        break;
      case BinaryOpKind::GT:
      case BinaryOpKind::GE:
      case BinaryOpKind::LT:
      case BinaryOpKind::LE:
      case BinaryOpKind::EQ:
      case BinaryOpKind::NE:
        analyze(lhs);
        analyze(rhs);

        p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::BOOL));
        break;
      case BinaryOpKind::MEMBER_ACCESS:
        analyze_member_access(ast_must_cast<MemberAccess>(p));
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  }

  void analyze_uop(ASTBase *_p) {
    auto p = ast_must_cast<UnaryOperator>(_p);
    auto rhs = p->get_rhs();
    analyze(rhs);

    switch (p->get_op()) {
      case UnaryOpKind::LNOT:
        p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::BOOL));
        break;
      case UnaryOpKind::BNOT:
        p->set_type(copy_ty(rhs->get_type()));
        break;
      case UnaryOpKind::ADDRESS_OF: {
        auto ty = copy_ty(rhs->get_type());
        p->set_type(ty->get_ptr_to());
        break;
      }
      case UnaryOpKind::PTR_DEREF: {
        auto *ty = rhs->get_type();
        if (!ty->is_ptr()) { report_error(ty, "Expect a pointer type"); }
        ty = copy_ty(ty->get_contained_ty());
        ty->set_is_lvalue(true);
        p->set_type(ty);
        break;
      }
      case UnaryOpKind::PLUS:
      case UnaryOpKind::MINUS: {
        /// unary plus/minus
        auto ty = copy_ty(rhs->get_type());
        ty->set_is_lvalue(false);
        p->set_type(ty);
        break;
      }
      default:
        TAN_ASSERT(false);
        break;
    }
  }

  void analyze_cast(Cast *p) {
    Expr *lhs = p->get_lhs();
    ASTBase *rhs = p->get_rhs();
    if (rhs->get_node_type() != ASTNodeType::TY) { report_error(rhs, "Expect a type"); }

    analyze(lhs);
    analyze(rhs);

    ASTType *ty = nullptr;
    switch (rhs->get_node_type()) {
      case ASTNodeType::ID:
        ty = _ctx->get_type_decl(ast_must_cast<Identifier>(rhs)->get_name())->get_type();
        if (!ty) {
          report_error(rhs, "Unknown type");
        }
        break;
      case ASTNodeType::TY:
        ty = ast_must_cast<ASTType>(rhs);
        break;
      default:
        report_error(lhs, "Invalid right-hand operand");
        break;
    }

    ty = copy_ty(ty);
    ty->set_is_lvalue(lhs->get_type()->is_lvalue());
    p->set_type(ty);
    // FIXME: check if the cast is valid
  }

  void analyze_assignment(Assignment *p) {
    Expr *rhs = p->get_rhs();
    analyze(rhs);

    auto lhs = p->get_lhs();
    ASTType *lhs_type = nullptr;
    switch (lhs->get_node_type()) {
      case ASTNodeType::ID:
        analyze(lhs);
        lhs_type = ast_must_cast<Identifier>(lhs)->get_type();
        break;
      case ASTNodeType::STRUCT_DECL:
      case ASTNodeType::VAR_DECL:
      case ASTNodeType::ARG_DECL:
      case ASTNodeType::ENUM_DECL:
      case ASTNodeType::BOP_OR_UOP:
      case ASTNodeType::UOP:
      case ASTNodeType::BOP:
        analyze(lhs);
        lhs_type = ast_must_cast<Expr>(lhs)->get_type();
        break;
      default:
        report_error(lhs, "Invalid left-hand operand");
    }

    /// if the type of lhs is not set, we deduce it
    /// NOTE: we only allow type deduction for declarations
    if (!lhs_type) {
      lhs_type = copy_ty(rhs->get_type());

      /// set type of lhs
      switch (lhs->get_node_type()) {
        case ASTNodeType::STRUCT_DECL:
        case ASTNodeType::VAR_DECL:
        case ASTNodeType::ARG_DECL:
        case ASTNodeType::ENUM_DECL:
          ast_must_cast<Decl>(lhs)->set_type(lhs_type);
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
      /// analyze again just to make sure
      analyze(lhs);
    }

    if (TypeSystem::CanImplicitCast(_ctx, lhs_type, rhs->get_type()) != 0) {
      report_error(p, "Cannot perform implicit type conversion");
    }
    p->set_type(lhs_type);
  }

  void analyze_func_call(ASTBase *_p) {
    auto p = ast_must_cast<FunctionCall>(_p);

    for (const auto &a: p->_args) {
      analyze(a);
    }

    FunctionDecl *callee = FunctionDecl::GetCallee(_ctx, p->get_name(), p->_args);
    p->_callee = callee;
    p->set_type(copy_ty(callee->get_ret_ty()));
  }

  void analyze_func_decl(ASTBase *_p) {
    FunctionDecl *p = ast_must_cast<FunctionDecl>(_p);

    /// add to external function table
    if (p->is_public() || p->is_external()) {
      ASTContext::AddPublicFunction(_ctx->_filename, p);
    }
    /// ...and to the internal function table
    _ctx->add_function(p);

    /// analyze return type
    analyze(p->get_ret_ty());

    _ctx->push_scope(); /// new scope

    /// analyze args
    size_t n = p->get_n_args();
    for (size_t i = 0; i < n; ++i) {
      analyze(p->get_arg_decls()[i]); /// args will be added to the scope here
    }

    /// function body
    if (!p->is_external()) {
      analyze(p->get_body());
    }

    _ctx->pop_scope(); /// pop scope
  }

  void analyze_import(ASTBase *_p) {
    auto p = ast_must_cast<Import>(_p);

    str file = p->get_filename();
    auto imported = Compiler::resolve_import(_ctx->_filename, file);
    if (imported.empty()) {
      report_error(p, "Cannot import: " + file);
    }

    /// it might be already parsed
    vector<FunctionDecl *> imported_functions = ASTContext::GetPublicFunctions(imported[0]);
    if (imported_functions.empty()) {
      Compiler::ParseFile(imported[0]);
      imported_functions = ASTContext::GetPublicFunctions(imported[0]);
    }

    /// import functions
    p->set_imported_funcs(imported_functions);
    for (FunctionDecl *f: imported_functions) {
      _ctx->add_function(f);
    }
  }

  void analyze_intrinsic(ASTBase *_p) {
    auto p = ast_must_cast<Intrinsic>(_p);
    auto c = p->get_sub();

    /// name
    str name;
    switch (c->get_node_type()) {
      case ASTNodeType::FUNC_CALL:
        name = ast_must_cast<FunctionCall>(c)->get_name();
        break;
      case ASTNodeType::ID:
        name = ast_must_cast<Identifier>(c)->get_name();
        break;
      default:
        TAN_ASSERT(false);
        break;
    }

    /// search for the intrinsic type
    auto q = Intrinsic::intrinsics.find(name);
    if (q == Intrinsic::intrinsics.end()) {
      report_error(p, "Invalid intrinsic");
    }
    p->set_intrinsic_type(q->second);

    auto void_type = ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::VOID);
    switch (p->get_intrinsic_type()) {
      case IntrinsicType::STACK_TRACE:
      case IntrinsicType::ABORT:
      case IntrinsicType::NOOP: {
        analyze(p->get_sub());
        p->set_type(void_type);
        break;
      }
      case IntrinsicType::LINENO: {
        auto sub = IntegerLiteral::Create(p->get_loc(), _sm->get_line(p->get_loc()), true);
        auto type = ASTType::CreateAndResolve(_ctx, p->get_loc(), TY_OR3(Ty::INT, Ty::UNSIGNED, Ty::BIT32));
        sub->set_type(type);
        p->set_type(type);
        p->set_sub(sub);
        break;
      }
      case IntrinsicType::FILENAME: {
        auto sub = StringLiteral::Create(p->get_loc(), _ctx->_filename);
        auto type = ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::STRING);
        sub->set_type(type);
        p->set_type(type);
        p->set_sub(sub);
        break;
      }
      case IntrinsicType::GET_DECL: {
        TAN_ASSERT(c->get_node_type() == ASTNodeType::FUNC_CALL);
        auto *func_call = ast_must_cast<FunctionCall>(c);
        if (func_call->get_n_args() != 1) {
          report_error(func_call, "Expect the number of args to be 1");
        }
        auto *target = func_call->get_arg(0);
        auto *source_str =
            ASTBuilder::CreateStringLiteral(_ctx, p->get_loc(), _ctx->get_source_manager()->get_source_code(target));
        p->set_sub(source_str);
        p->set_type(source_str->get_type());
        break;
      }
      case IntrinsicType::COMP_PRINT: {
        p->set_type(void_type);

        auto func_call = ast_must_cast<FunctionCall>(c);
        auto args = func_call->_args;

        if (args.size() != 1 || args[0]->get_node_type() != ASTNodeType::STRING_LITERAL) {
          report_error(p, "Invalid call to compprint, one argument with type 'str' required");
        }
        str msg = ast_must_cast<StringLiteral>(args[0])->get_value();
        std::cout << fmt::format("Message ({}): {}\n", _ctx->get_source_location_str(p), msg);
        break;
      }
      default:
        report_error(p, "Unknown intrinsic");
    }
  }

  void analyze_string_literal(ASTBase *_p) {
    auto p = ast_must_cast<StringLiteral>(_p);
    p->set_value(_sm->get_token_str(p->get_loc()));
    p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::STRING));
  }

  void analyze_char_literal(ASTBase *_p) {
    auto p = ast_must_cast<CharLiteral>(_p);

    p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::CHAR, {}));
    p->set_value(static_cast<uint8_t>(_sm->get_token_str(p->get_loc())[0]));
  }

  void analyze_integer_literal(ASTBase *_p) {
    auto p = ast_must_cast<IntegerLiteral>(_p);
    auto tyty = Ty::INT;

    if (_ctx->get_source_manager()->get_token(p->get_loc())->is_unsigned()) {
      tyty = TY_OR(tyty, Ty::UNSIGNED);
    }
    p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), tyty));
  }

  void analyze_bool_literal(ASTBase *_p) {
    auto p = ast_must_cast<BoolLiteral>(_p);
    p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::BOOL));
  }

  void analyze_float_literal(ASTBase *_p) {
    auto p = ast_must_cast<FloatLiteral>(_p);
    p->set_type(ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::FLOAT));
  }

  void analyze_array_literal(ASTBase *_p) {
    auto p = ast_must_cast<ArrayLiteral>(_p);

    // TODO: restrict array element type to be the same
    vector<ASTType *> sub_tys{};
    auto elements = p->get_elements();
    sub_tys.reserve(elements.size());
    std::for_each(elements.begin(), elements.end(), [&sub_tys, this](Expr *e) {
      analyze(e);
      sub_tys.push_back(e->get_type());
    });

    ASTType *ty = ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::ARRAY, sub_tys);
    ty->set_array_size(elements.size());
    p->set_type(ty);
  }

  void analyze_member_access(MemberAccess *p) {
    Expr *lhs = p->get_lhs();
    analyze(lhs);
    Expr *rhs = p->get_rhs();

    if (p->_access_type == MemberAccess::MemberAccessDeref) { /// pointer dereference
      analyze(rhs);
      auto ty = lhs->get_type();
      TAN_ASSERT(ty->is_ptr());
      ty = copy_ty(ty->get_contained_ty());
      ty->set_is_lvalue(true);
      p->set_type(ty);
    } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
      analyze(rhs);
      auto ty = lhs->get_type();
      if (!ty->is_ptr()) {
        report_error(p, "Expect a pointer or an array");
      }

      ty = copy_ty(ty->get_contained_ty());
      ty->set_is_lvalue(true);

      p->set_type(ty);
      if (rhs->get_node_type() == ASTNodeType::INTEGER_LITERAL) {
        auto size_node = ast_must_cast<IntegerLiteral>(rhs);
        auto size = size_node->get_value(); // underflow is ok
        if (rhs->get_type()->is_array() && size >= lhs->get_type()->get_array_size()) {
          report_error(p,
              fmt::format("Index {} out of bound, the array size is {}",
                  std::to_string(size),
                  std::to_string(lhs->get_type()->get_array_size())));
        }
      }
    } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable or enum
      analyze(rhs);
      if (lhs->get_type()->is_enum()) {
        // TODO: Member access of enums
        TAN_ASSERT(false);
      } else {
        p->_access_type = MemberAccess::MemberAccessMemberVariable;
        if (!lhs->get_type()->is_lvalue() && !lhs->get_type()->is_ptr()) {
          report_error(p, "Invalid left-hand operand");
        }

        str m_name = ast_must_cast<Identifier>(rhs)->get_name();
        ASTType *struct_ty = nullptr;
        /// auto dereference pointers
        if (lhs->get_type()->is_ptr()) {
          struct_ty = lhs->get_type()->get_contained_ty();
        } else {
          struct_ty = lhs->get_type();
        }
        struct_ty = struct_ty->get_canonical_type(); /// resolve type references
        if (struct_ty->get_ty() != Ty::STRUCT) {
          report_error(lhs, "Expect a struct type");
        }

        StructDecl *struct_decl = ast_must_cast<StructDecl>(_ctx->get_type_decl(struct_ty->get_type_name()));
        p->_access_idx = struct_decl->get_struct_member_index(m_name);
        auto ty = copy_ty(struct_decl->get_struct_member_ty(p->_access_idx));
        ty->set_is_lvalue(true);
        p->set_type(ty);
      }
    } else if (p->_access_type == MemberAccess::MemberAccessMemberFunction) { /// method call
      if (!lhs->get_type()->is_lvalue() && !lhs->get_type()->is_ptr()) {
        report_error(p, "Method calls require left-hand operand to be an lvalue or a pointer");
      }
      auto func_call = ast_cast<FunctionCall>(rhs);
      if (!func_call) {
        report_error(rhs, "Expect a function call");
      }

      /// get address of the struct instance
      if (lhs->get_type()->is_lvalue() && !lhs->get_type()->is_ptr()) {
        Expr *tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs->get_loc(), lhs);
        analyze(tmp);
        func_call->_args.insert(func_call->_args.begin(), tmp);
      } else {
        func_call->_args.insert(func_call->_args.begin(), lhs);
      }

      /// postpone analysis of FUNC_CALL until now
      analyze(rhs);
      p->set_type(copy_ty(rhs->get_type()));
    } else {
      report_error(p, "Invalid right-hand operand");
    }
  }

  void analyze_struct_decl(ASTBase *_p) {
    auto p = ast_must_cast<StructDecl>(_p);

    ASTType *ty = nullptr;
    str struct_name = p->get_name();
    _ctx->add_type_decl(struct_name, p);

    /// check if struct name is in conflicts of variable/function names
    /// or if there's a forward declaration
    ASTType *prev = _ctx->get_type_decl(struct_name)->get_type();
    if (prev) {
      if (prev->get_node_type() != ASTNodeType::STRUCT_DECL) { /// fwd decl
        ty = std::move(prev);
      } else { /// conflict
        report_error(p, "Cannot redeclare type as a struct");
      }
    } else { /// no fwd decl
      ty = ASTType::Create(_ctx, p->get_loc());
      ty->set_ty(Ty::STRUCT);
      ty->set_constructor(StructConstructor::Create(ty));
      _ctx->add_type_decl(struct_name, p); /// add self to current scope
    }
    ty->set_type_name(struct_name);

    /// resolve member names and types
    auto member_decls = p->get_member_decls();  // size is 0 if no struct body
    size_t n = member_decls.size();
    ty->get_sub_types().reserve(n);
    for (size_t i = 0; i < n; ++i) {
      Expr *m = member_decls[i];
      analyze(m);

      if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
        /// fill members
        str name = ast_must_cast<VarDecl>(m)->get_name();
        p->set_member_index(name, i);
        ty->get_sub_types().push_back(m->get_type());
      }
        /// member variable with an initial value
      else if (m->get_node_type() == ASTNodeType::ASSIGN) {
        auto bm = ast_must_cast<Assignment>(m);
        auto init_val = bm->get_rhs();

        if (bm->get_lhs()->get_node_type() != ASTNodeType::VAR_DECL) {
          report_error(bm, "Expect a member variable declaration");
        }
        auto decl = ast_must_cast<VarDecl>(bm->get_lhs());

        /// fill members
        ty->get_sub_types().push_back(decl->get_type());
        p->set_member_index(decl->get_name(), i);

        /// initial values
        if (!init_val->is_comptime_known()) {
          report_error(p, "Initial value of a member variable must be compile-time known");
        }
        auto *ctr = cast_ptr<StructConstructor>(ty->get_constructor());
        ctr->get_member_constructors().push_back(BasicConstructor::Create(ast_must_cast<CompTimeExpr>(init_val)));
      } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) {
        auto f = ast_must_cast<FunctionDecl>(m);

        /// fill members
        ty->get_sub_types().push_back(f->get_type());
        p->set_member_index(f->get_name(), i);
      } else {
        report_error(p, "Invalid struct member");
      }
    }
    resolve_ty(ty);
    p->set_type(ty);
  }

  void analyze_loop(ASTBase *_p) {
    auto *p = ast_must_cast<Loop>(_p);
    analyze(p->get_predicate());

    auto *prev_loop = _ctx->get_current_loop();
    _ctx->set_current_loop(p);
    analyze(p->get_body());
    _ctx->set_current_loop(prev_loop);
  }

  void analyze_break_or_continue(ASTBase *_p) {
    auto *p = ast_must_cast<BreakContinue>(_p);
    Loop *loop = _ctx->get_current_loop();
    if (!loop) {
      report_error(p, "Break or continue must be inside a loop");
    }
    p->set_parent_loop(loop);
  }

  void analyze_enum_decl(ASTBase *_p) {
    EnumDecl *p = ast_must_cast<EnumDecl>(_p);

    int64_t val = 0;
    size_t i = 0;
    size_t n = p->get_elements().size();
    vector<str> names(n);
    vector<int64_t> values(n);
    for (const auto &e : p->get_elements()) {
      if (e->get_node_type() == ASTNodeType::ID) {
        names[i] = ast_must_cast<Identifier>(e)->get_name();
        values[i] = val;
      } else if (e->get_node_type() == ASTNodeType::ASSIGN) {
        auto *assignment = ast_must_cast<Assignment>(e);
        auto *_lhs = assignment->get_lhs();
        auto *_rhs = assignment->get_rhs();

        auto *lhs = ast_cast<ASTNamed>(_lhs);
        if (!lhs) { report_error(_lhs, "Expect a name"); }

        if (_rhs->get_node_type() != ASTNodeType::INTEGER_LITERAL) { report_error(_rhs, "Expect an integer literal"); }
        auto *rhs = ast_cast<IntegerLiteral>(_rhs);
        TAN_ASSERT(rhs);

        names[i] = lhs->get_name();
        values[i] = val = rhs->get_value();
      }
      ++val;
      ++i;
    }
    p->set_names(names);
    p->set_values(values);
  }
};

void Analyzer::analyze(ASTBase *p) { _analyzer_impl->analyze(p); }

Analyzer::Analyzer(ASTContext *cs) { _analyzer_impl = new AnalyzerImpl(cs); }

Analyzer::~Analyzer() { if (_analyzer_impl) { delete _analyzer_impl; }}

}
