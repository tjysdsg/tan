#include "analyzer.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_builder.h"
#include "src/ast/constructor.h"
#include "src/ast/type.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/ast/intrinsic.h"
#include "src/analysis/type_system.h"
#include "src/scope.h"
#include "src/ast/ast_context.h"
#include "compiler.h"
#include "token.h"
#include <iostream>
#include <csetjmp>

namespace tanlang {

class AnalyzerImpl {
public:
  explicit AnalyzerImpl(ASTContext *cs) : _ctx(cs), _sm(cs->get_source_manager()) {}

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
      case ASTNodeType::TY:
        analyze_ty(p);
        break;
      default:
        TAN_ASSERT(false);
    }
  }

private:
  ASTContext *_ctx = nullptr;
  SourceManager *_sm = nullptr;

private:
  [[noreturn]] void report_error(ASTBase *p, const str &message) {
    Error err(_ctx->_filename, _sm->get_token(p->loc()), message);
    err.raise();
  }

  void analyze_ty(ASTBase *_p) {
    auto *p = ast_must_cast<Type>(_p);
    // TODO IMPORTANT
  }

  void analyze_id(ASTBase *_p) {
    auto p = ast_must_cast<Identifier>(_p);
    auto *referred = _ctx->get_decl(p->get_name());
    if (referred) { /// refers to a variable
      p->set_var_ref(VarRef::Create(p->loc(), p->get_name(), referred));
      p->set_type(referred->get_type());
    } else if (_ctx->get_type_decl(p->get_name())) { /// or type ref
      auto *ty = _ctx->get_type_decl(p->get_name())->get_type();
      p->set_type_ref(ty);
      p->set_type(ty);
    } else {
      report_error(p, "Unknown identifier");
    }
  }

  void analyze_parenthesis(ASTBase *_p) {
    auto p = ast_must_cast<Parenthesis>(_p);

    analyze(p->get_sub());

    p->set_type(p->get_sub()->get_type());
  }

  void analyze_if(ASTBase *_p) {
    auto p = ast_must_cast<If>(_p);

    size_t n = p->get_num_branches();
    for (size_t i = 0; i < n; ++i) {
      auto cond = p->get_predicate(i);
      if (cond) { /// can be nullptr, meaning an "else" branch
        analyze(cond);

        if (!TypeSystem::CanImplicitlyConvert(cond->get_type(), PrimitiveType::GetBoolType())) {
          report_error(cond, "Cannot implicitly convert expression to bool");
        }
      }

      analyze(p->get_branch(i));
    }
  }

  void analyze_var_decl(ASTBase *_p) {
    auto p = ast_must_cast<VarDecl>(_p);

    /// analyze type if specified
    Type *ty = p->get_type();
    if (ty) {
      analyze(ty);
    }

    _ctx->add_decl(p->get_name(), p);
  }

  void analyze_arg_decl(ASTBase *_p) {
    auto p = ast_must_cast<ArgDecl>(_p);
    Type *ty = p->get_type();
    analyze(ty);
    _ctx->add_decl(p->get_name(), p);
  }

  void analyze_ret(ASTBase *_p) {
    // TODO: check if return type is the same as the function return type
    auto p = ast_must_cast<Return>(_p);
    auto *rhs = p->get_rhs();
    if (rhs) { analyze(rhs); }
  }

  void analyze_stmt(ASTBase *_p) {
    auto p = ast_must_cast<CompoundStmt>(_p);

    if (p->is_new_scope()) { _ctx->push_scope(); }

    for (const auto &c: p->get_children()) {
      analyze(c);
    }

    if (p->is_new_scope()) { _ctx->pop_scope(); }
  }

  void analyze_bop_or_uop(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOrUnary>(_p);
    analyze(p->get_expr_ptr());
  }

  void analyze_bop(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOperator>(_p);
    Expr *lhs = p->get_lhs();
    Expr *rhs = p->get_rhs();

    /// NOTE: do not analyze lhs and rhs just yet, because analyze_assignment
    /// and analyze_member_access have their own ways of analyzing

    // TODO: IMPORTANT determine which operand's type should the other one implicitly convert to
    switch (p->get_op()) {
      case BinaryOpKind::SUM:
      case BinaryOpKind::SUBTRACT:
      case BinaryOpKind::MULTIPLY:
      case BinaryOpKind::DIVIDE:
      case BinaryOpKind::MOD: {
        analyze(lhs);
        analyze(rhs);

        if (!TypeSystem::CanImplicitlyConvert(lhs->get_type(), rhs->get_type())
            && !TypeSystem::CanImplicitlyConvert(rhs->get_type(), lhs->get_type())) {
          report_error(p, "Cannot implicitly convert between two expressions");
        }

        p->set_type(lhs->get_type());
        break;
      }
      case BinaryOpKind::BAND:
      case BinaryOpKind::LAND:
      case BinaryOpKind::BOR:
      case BinaryOpKind::LOR:
      case BinaryOpKind::XOR: {
        analyze(lhs);
        analyze(rhs);

        // check if both operators are bool
        auto *bool_type = PrimitiveType::GetBoolType();
        if (!TypeSystem::CanImplicitlyConvert(lhs->get_type(), bool_type)) {
          report_error(p, "Cannot implicitly convert lhs to bool");
        }
        if (!TypeSystem::CanImplicitlyConvert(rhs->get_type(), bool_type)) {
          report_error(p, "Cannot implicitly convert rhs to bool");
        }

        p->set_type(bool_type);
        break;
      }
      case BinaryOpKind::GT:
      case BinaryOpKind::GE:
      case BinaryOpKind::LT:
      case BinaryOpKind::LE:
      case BinaryOpKind::EQ:
      case BinaryOpKind::NE:
        analyze(lhs);
        analyze(rhs);
        if (!TypeSystem::CanImplicitlyConvert(lhs->get_type(), rhs->get_type())) {
          report_error(p, "Cannot implicitly convert between lhs and rhs");
        }
        p->set_type(PrimitiveType::GetBoolType());
        break;
      case BinaryOpKind::MEMBER_ACCESS:
        analyze_member_access(ast_must_cast<MemberAccess>(p));
        break;
      default:
        TAN_ASSERT(false);
    }
  }

  void analyze_uop(ASTBase *_p) {
    auto *p = ast_must_cast<UnaryOperator>(_p);
    auto *rhs = p->get_rhs();
    analyze(rhs);

    auto *rhs_type = rhs->get_type();
    switch (p->get_op()) {
      case UnaryOpKind::LNOT:
        if (!rhs_type->is_bool()) { report_error(rhs, "Expect a bool type"); }
        p->set_type(PrimitiveType::GetBoolType());
        break;
      case UnaryOpKind::BNOT:
        if (!rhs_type->is_int()) { report_error(rhs, "Expect an integer type"); }
        p->set_type(rhs_type);
        break;
      case UnaryOpKind::ADDRESS_OF:
        p->set_type(Type::GetPointerType(rhs_type));
        break;
      case UnaryOpKind::PTR_DEREF:
        if (!rhs_type->is_pointer()) { report_error(rhs, "Expect a pointer type"); }
        TAN_ASSERT(rhs->is_lvalue());
        p->set_lvalue(true);
        p->set_type(ast_must_cast<PointerType>(rhs_type)->get_pointee());
        break;
      case UnaryOpKind::PLUS:
      case UnaryOpKind::MINUS: /// unary plus/minus
        if (!(rhs_type->is_int() || rhs_type->is_float())) { report_error(rhs, "Expect an numerical type"); }
        p->set_type(rhs_type);
        break;
      default:
        TAN_ASSERT(false);
    }
  }

  void analyze_cast(Cast *p) {
    Expr *lhs = p->get_lhs();
    ASTBase *rhs = p->get_rhs();
    analyze(lhs);
    analyze(rhs);

    Type *ty = nullptr;
    switch (rhs->get_node_type()) {
      case ASTNodeType::ID:
        ty = ast_must_cast<Identifier>(rhs)->get_type();
        if (!ty) { report_error(rhs, "Unknown type"); }
        break;
      case ASTNodeType::TY:
        ty = ast_must_cast<Type>(rhs);
        break;
      default:
        report_error(lhs, "Invalid right-hand operand");
        break;
    }

    p->set_type(ty);
    // FIXME: check if the cast is valid
  }

  void analyze_assignment(Assignment *p) {
    Expr *rhs = p->get_rhs();
    analyze(rhs);

    auto lhs = p->get_lhs();
    Type *lhs_type = nullptr;
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
      lhs_type = rhs->get_type();

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

    p->set_lvalue(true);

    if (!TypeSystem::CanImplicitlyConvert(rhs->get_type(), lhs_type)) {
      report_error(p, "Cannot implicitly cast rhs to lhs");
    }
    p->set_type(lhs_type);
  }

  void analyze_func_call(ASTBase *_p) {
    auto p = ast_must_cast<FunctionCall>(_p);

    for (const auto &a: p->_args) {
      analyze(a);
    }

    FunctionDecl *callee = FunctionDecl::GetCallee(_ctx, p);
    p->_callee = callee;
    p->set_type(callee->get_ret_ty());
  }

  void analyze_func_decl(ASTBase *_p) {
    auto *p = ast_must_cast<FunctionDecl>(_p);

    /// add_decl to external function table
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
        name = p->get_name();
        break;
    }
    TAN_ASSERT(!name.empty());

    /// search for the intrinsic type
    auto q = Intrinsic::intrinsics.find(name);
    if (q == Intrinsic::intrinsics.end()) {
      report_error(p, "Invalid intrinsic");
    }
    p->set_intrinsic_type(q->second);

    auto void_type = Type::GetVoidType();
    switch (p->get_intrinsic_type()) {
      case IntrinsicType::STACK_TRACE:
      case IntrinsicType::ABORT:
      case IntrinsicType::NOOP: {
        analyze(p->get_sub());
        p->set_type(void_type);
        break;
      }
      case IntrinsicType::LINENO: {
        auto sub = IntegerLiteral::Create(p->loc(), _sm->get_line(p->loc()), true);
        auto type = PrimitiveType::GetIntegerType(32, true);
        sub->set_type(type);
        p->set_type(type);
        p->set_sub(sub);
        break;
      }
      case IntrinsicType::FILENAME: {
        auto sub = StringLiteral::Create(p->loc(), _ctx->_filename);
        auto type = Type::GetStringType();
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
            ASTBuilder::CreateStringLiteral(p->loc(), _ctx->get_source_manager()->get_source_code(target));
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
      case IntrinsicType::TEST_COMP_ERROR: {
        // FIXME: memory leaks
        bool error_catched = false;
        std::jmp_buf buf;
        if (setjmp(buf) > 0) {
          error_catched = true;
        } else {
          auto error_catcher = ErrorCatcher((const ErrorCatcher::callback_t &) [&](str) {
            longjmp(buf, 1);
          });
          Error::CatchErrors(&error_catcher);
          analyze(p->get_sub());
        }

        Error::ResetErrorCatcher();
        if (!error_catched) {
          report_error(p, "Expect a compile error");
        }
        break;
      }
      default:
        report_error(p, "Unknown intrinsic");
    }
  }

  void analyze_string_literal(ASTBase *_p) {
    auto p = ast_must_cast<StringLiteral>(_p);
    p->set_value(_sm->get_token_str(p->loc()));
    p->set_type(Type::GetStringType());
  }

  void analyze_char_literal(ASTBase *_p) {
    auto p = ast_must_cast<CharLiteral>(_p);
    p->set_type(Type::GetCharType());
    p->set_value(static_cast<uint8_t>(_sm->get_token_str(p->loc())[0]));
  }

  void analyze_integer_literal(ASTBase *_p) {
    auto p = ast_must_cast<IntegerLiteral>(_p);

    Type *ty;
    if (_ctx->get_source_manager()->get_token(p->loc())->is_unsigned()) {
      ty = Type::GetIntegerType(32, true);
    } else {
      ty = Type::GetIntegerType(32, false);
    }
    p->set_type(ty);
  }

  void analyze_bool_literal(ASTBase *_p) {
    auto p = ast_must_cast<BoolLiteral>(_p);
    p->set_type(Type::GetBoolType());
  }

  void analyze_float_literal(ASTBase *_p) {
    auto p = ast_must_cast<FloatLiteral>(_p);
    p->set_type(Type::GetFloatType(32));
  }

  void analyze_array_literal(ASTBase *_p) {
    auto p = ast_must_cast<ArrayLiteral>(_p);

    // TODO IMPORTANT: find the type that all elements can implicitly convert to
    //  for example: [1, 2.2, 3u] has element type float
    auto elements = p->get_elements();
    Type *element_type = nullptr;
    for (auto *e: elements) {
      analyze(e);
      if (!element_type) { element_type = e->get_type(); }
      if (!TypeSystem::CanImplicitlyConvert(e->get_type(), element_type)) {
        report_error(p, "All elements in an array must have the same type");
      }
    }

    TAN_ASSERT(element_type);
    p->set_type(Type::GetArrayType(element_type, (int) elements.size()));
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_member_func_call(MemberAccess *p, Expr *lhs, FunctionCall *rhs) {
    if (!lhs->is_lvalue() && !lhs->get_type()->is_pointer()) {
      report_error(p, "Invalid member function call");
    }

    /// get address of the struct instance
    if (lhs->is_lvalue() && !lhs->get_type()->is_pointer()) {
      Expr *tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs->loc(), lhs);
      analyze(tmp);
      rhs->_args.insert(rhs->_args.begin(), tmp);
    } else {
      rhs->_args.insert(rhs->_args.begin(), lhs);
    }

    /// postpone analysis of FUNC_CALL until now
    analyze(rhs);
    p->set_type(rhs->get_type());
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_bracket_access(MemberAccess *p, Expr *lhs, Expr *rhs) {
    analyze(rhs);

    if (!lhs->is_lvalue()) {
      report_error(p, "Expect lhs to be an lvalue");
    }

    auto *lhs_type = lhs->get_type();
    if (!(lhs_type->is_pointer() || lhs_type->is_array() || lhs_type->is_string())) {
      report_error(p, "Expect a type that supports bracket access");
    }
    if (!rhs->get_type()->is_int()) { report_error(rhs, "Expect an integer"); }

    Type *sub_type = nullptr;
    if (lhs_type->is_pointer()) {
      sub_type = ast_must_cast<PointerType>(lhs_type)->get_pointee();
    } else if (lhs_type->is_array()
        && rhs->get_node_type() == ASTNodeType::INTEGER_LITERAL) { /// check if array index is out-of-bound
      uint64_t size = ast_must_cast<IntegerLiteral>(rhs)->get_value();
      auto *array_type = ast_must_cast<ArrayType>(lhs_type);
      sub_type = array_type->get_element_type();
      if (lhs->get_type()->is_array() && (int) size >= array_type->get_size()) {
        report_error(p,
            fmt::format("Index {} out of bound, the array size is {}",
                std::to_string(size),
                std::to_string(array_type->get_size())));
      }
    } else if (lhs_type->is_string()) {
      sub_type = Type::GetCharType();
    }

    p->set_type(sub_type);
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_enum_member_access(MemberAccess *p, Expr *lhs, Expr *rhs) {
    p->set_type(lhs->get_type());

    str enum_name = ast_must_cast<Identifier>(lhs)->get_name();
    auto *enum_decl = ast_must_cast<EnumDecl>(_ctx->get_type_decl(enum_name));

    /// enum element
    if (rhs->get_node_type() != ASTNodeType::ID) { report_error(rhs, "Unknown enum element"); }
    str name = ast_must_cast<Identifier>(rhs)->get_name();
    if (!enum_decl->contain_element(name)) { report_error(rhs, "Unknown enum element"); }
  }

  /// ASSUMES lhs has been already analyzed, while rhs has not
  void analyze_member_access_member_variable(MemberAccess *p, Expr *lhs, Expr *rhs) {
    /* TODO IMPORTANT: FIX THIS
    analyze(rhs);

    str m_name = ast_must_cast<Identifier>(rhs)->get_name();
    Type *struct_ty = nullptr;
    /// auto dereference pointers
    if (lhs->get_type()->is_pointer()) {
      struct_ty = ast_must_cast<PointerType>(lhs->get_type())->get_pointee();
    } else {
      struct_ty = lhs->get_type();
    }
    struct_ty = struct_ty->get_canonical_type(); /// resolve type references
    if (struct_ty->get_ty() != Ty::STRUCT) { report_error(lhs, "Expect a struct type"); }

    auto *struct_decl = ast_must_cast<StructDecl>(_ctx->get_type_decl(struct_ty->get_type_name()));
    p->_access_idx = struct_decl->get_struct_member_index(m_name);
    auto ty = copy_ty(struct_decl->get_struct_member_ty(p->_access_idx));
    p->set_type(ty);
    */
    TAN_ASSERT(false);
  }

  void analyze_member_access(MemberAccess *p) {
    Expr *lhs = p->get_lhs();
    analyze(lhs);
    Expr *rhs = p->get_rhs();

    if (rhs->get_node_type() == ASTNodeType::FUNC_CALL) { /// method call
      p->_access_type = MemberAccess::MemberAccessMemberFunction;
      auto func_call = ast_must_cast<FunctionCall>(rhs);
      analyze_member_func_call(p, lhs, func_call);
    } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
      analyze_bracket_access(p, lhs, rhs);
    } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable or enum
      if (lhs->get_type()->is_enum()) {
        p->_access_type = MemberAccess::MemberAccessEnumValue;
        analyze_enum_member_access(p, lhs, rhs);
      } else { /// member variable
        p->_access_type = MemberAccess::MemberAccessMemberVariable;
        analyze_member_access_member_variable(p, lhs, rhs);
      }
    } else {
      report_error(p, "Invalid right-hand operand");
    }
  }

  void analyze_struct_decl(ASTBase *_p) {
    /* TODO IMPORTANT: FIX THIS
    auto p = ast_must_cast<StructDecl>(_p);

    Type *ty = nullptr;
    /// check if struct name is in conflicts of variable/function names
    /// or if there's a forward declaration
    str struct_name = p->get_name();
    auto *prev_decl = _ctx->get_type_decl(struct_name);
    if (prev_decl) {
      if (!(prev_decl->get_node_type() == ASTNodeType::STRUCT_DECL
          && ast_must_cast<StructDecl>(prev_decl)->is_forward_decl())) { /// conflict
        report_error(p, "Cannot redeclare type as a struct");
      }
    } else {
      ty = Type::Create(_ctx, p->loc());
      ty->set_ty(Ty::STRUCT);
      ty->set_constructor(StructConstructor::Create(ty));
      _ctx->add_type_decl(struct_name, p); /// add_decl self to current scope
    }
    ty->set_type_name(struct_name);

    /// register type to context
    _ctx->add_type_decl(struct_name, p);

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
     */
    TAN_ASSERT(false);
  }

  void analyze_loop(ASTBase *_p) {
    auto *p = ast_must_cast<Loop>(_p);
    analyze(p->get_predicate());

    _ctx->push_scope();
    _ctx->set_current_loop(p);
    analyze(p->get_body());
    _ctx->pop_scope(); /// current loop (p) is discarded after popping
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
    /* TODO IMPORTANT: FIX THIS
    auto *p = ast_must_cast<EnumDecl>(_p);

    /// add_decl the enum type to context
    auto *ty = Type::GetEnumType(_ctx, p->loc(), p->get_name());
    TypeSystem::SetDefaultConstructor(_ctx, ty);
    p->set_type(ty);
    _ctx->add_type_decl(p->get_name(), p);

    /// get element names and types
    int64_t val = 0;
    size_t i = 0;
    for (const auto &e: p->get_elements()) {
      if (e->get_node_type() == ASTNodeType::ID) {
        p->set_value(ast_must_cast<Identifier>(e)->get_name(), val);
      } else if (e->get_node_type() == ASTNodeType::ASSIGN) {
        auto *assignment = ast_must_cast<Assignment>(e);
        auto *_lhs = assignment->get_lhs();
        auto *_rhs = assignment->get_rhs();

        auto *lhs = ast_cast<ASTNamed>(_lhs);
        if (!lhs) { report_error(_lhs, "Expect a name"); }

        if (_rhs->get_node_type() != ASTNodeType::INTEGER_LITERAL) { report_error(_rhs, "Expect an integer literal"); }
        auto *rhs = ast_cast<IntegerLiteral>(_rhs);
        TAN_ASSERT(rhs);

        val = (int64_t) rhs->get_value();
        p->set_value(lhs->get_name(), val);
      }
      ++val;
      ++i;
    }
    */
    TAN_ASSERT(false);
  }
};

void Analyzer::analyze(ASTBase *p) { _analyzer_impl->analyze(p); }

Analyzer::Analyzer(ASTContext *cs) { _analyzer_impl = new AnalyzerImpl(cs); }

Analyzer::~Analyzer() { delete _analyzer_impl; }

}
