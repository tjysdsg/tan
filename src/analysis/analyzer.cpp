#include "analyzer.h"
#include "base.h"
#include "src/analysis/ast_helper.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/ast/intrinsic.h"
#include "src/analysis/type_system.h"
#include "token.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/common.h"
#include <iostream>
#include <fmt/core.h>

namespace tanlang {

class AnalyzerImpl {
public:
  AnalyzerImpl(CompilerSession *cs) : _cs(cs), _h(ASTHelper(cs)) {}

  void analyze(ASTBase *p) {
    TAN_ASSERT(p);
    p->set_scope(_cs->get_current_scope());

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
        // TODO: cs->set_current_loop(pl) // case ASTNodeType::LOOP:
        // TODO: cs->get_current_loop() // case ASTNodeType::BREAK (or CONTINUE):
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
        // TODO: Analysis of enum types and values
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
      case ASTNodeType::CONTINUE:
      case ASTNodeType::BREAK:
        break;
      case ASTNodeType::BOP_OR_UOP:
        analyze_bop_or_uop(p);
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  }

private:
  ASTType *copy_ty(ASTType *p) const { return new ASTType(*p); }

  void resolve_ty(ASTType *p) const {
    TypeSystem::ResolveTy(_cs, p);
  }

  [[noreturn]] void report_error(ASTBase *p, const str &message) {
    tanlang::report_error(_cs->_filename, p->get_token(), message);
  }

  void analyze_uop(ASTBase *_p) {
    auto p = ast_must_cast<UnaryOperator>(_p);
    auto rhs = p->get_rhs();
    analyze(rhs);

    switch (p->get_op()) {
      case UnaryOpKind::LNOT:
        p->set_type(ASTType::Create(_cs, Ty::BOOL));
        break;
      case UnaryOpKind::BNOT:
        p->set_type(copy_ty(rhs->get_type()));
        break;
      case UnaryOpKind::ADDRESS_OF: {
        auto ty = copy_ty(rhs->get_type());
        p->set_type(_h.get_ptr_to(ty));
        break;
      }
      case UnaryOpKind::PLUS:
      case UnaryOpKind::MINUS: {
        /// unary plus/minus
        auto ty = copy_ty(rhs->get_type());
        ty->_is_lvalue = false;
        p->set_type(ty);
        break;
      }
      default:
        TAN_ASSERT(false);
        break;
    }
  }

  void analyze_id(ASTBase *_p) {
    auto p = ast_must_cast<Identifier>(_p);
    auto referred = _cs->get(p->get_name());
    if (!referred) {
      report_error(p, "Unknown identifier");
    }

    auto declared = ast_cast<Decl>(referred);
    if (!declared) {
      report_error(p, "Invalid identifier");
    }

    p->_referred = declared;
    auto ty = copy_ty(declared->get_type());
    ty->_is_lvalue = true;
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
        if (0 != TypeSystem::CanImplicitCast(_cs, ASTType::Create(_cs, Ty::BOOL), cond->get_type())) {
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
      ty->_is_lvalue = true;
      analyze(ty);
    }

    _cs->add(p->get_name(), p);
  }

  void analyze_arg_decl(ASTBase *_p) {
    auto p = ast_must_cast<ArgDecl>(_p);
    ASTType *ty = p->get_type();
    ty->_is_lvalue = true;
    analyze(ty);
    _cs->add(p->get_name(), p);
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

  void analyze_cast(Cast *p) {
    Expr *lhs = p->get_lhs();
    ASTBase *rhs = p->get_rhs();
    analyze(lhs);
    analyze(rhs);

    ASTType *ty = nullptr;
    switch (rhs->get_node_type()) {
      case ASTNodeType::ID:
        ty = _cs->get_type_decl(ast_must_cast<Identifier>(rhs)->get_name())->get_type();
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
    ty->_is_lvalue = lhs->get_type()->_is_lvalue;
    p->set_type(ty);
    // FIXME: check if can explicit cast
    // if (TypeSystem::CanImplicitCast(_cs, np->_type, _h.get_ty(p->get_child_at(0))) != 0) {
    //   report_error(p, "Cannot perform implicit type conversion");
    // }
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

    if (TypeSystem::CanImplicitCast(_cs, lhs_type, rhs->get_type()) != 0) {
      report_error(p, "Cannot perform implicit type conversion");
    }
    p->set_type(lhs_type);
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

        int i = TypeSystem::CanImplicitCast(_cs, lhs->get_type(), rhs->get_type());
        if (i == -1) {
          report_error(p, "Cannot perform implicit type conversion");
        }

        size_t dominant_idx = static_cast<size_t>(i);
        p->set_dominant_idx(dominant_idx);
        ASTType *ty = copy_ty(dominant_idx == 0 ? lhs->get_type() : rhs->get_type());
        ty->_is_lvalue = false;
        p->set_type(ty);
        break;
      }
      case BinaryOpKind::BAND:
      case BinaryOpKind::LAND:
      case BinaryOpKind::BOR:
      case BinaryOpKind::LOR:
      case BinaryOpKind::XOR:
        // TODO: implement the analysis of the above operators
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

        p->set_type(ASTType::Create(_cs, Ty::BOOL));
        break;
      case BinaryOpKind::MEMBER_ACCESS:
        analyze_member_access(ast_must_cast<MemberAccess>(p));
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
  }

  void analyze_func_call(ASTBase *_p) {
    auto p = ast_must_cast<FunctionCall>(_p);

    for (const auto &a: p->_args) {
      analyze(a);
    }

    FunctionDecl *callee = FunctionDecl::GetCallee(_cs, p->get_name(), p->_args);
    p->_callee = callee;
    p->set_type(copy_ty(callee->get_ret_ty()));
  }

  void analyze_func_decl(ASTBase *_p) {
    FunctionDecl *p = ast_must_cast<FunctionDecl>(_p);

    /// add to external function table
    if (p->is_public() || p->is_external()) {
      CompilerSession::AddPublicFunction(_cs->_filename, p);
    }
    /// ...and to the internal function table
    _cs->add_function(p);

    /// analyze return type
    analyze(p->get_ret_ty());

    _cs->push_scope(); /// new scope

    /// analyze args
    size_t n = p->get_n_args();
    for (size_t i = 0; i < n; ++i) {
      analyze(p->get_arg_decls()[i]); /// args will be added to the scope here
    }

    /// function body
    if (!p->is_external()) {
      analyze(p->get_body());
    }

    _cs->pop_scope(); /// pop scope
  }

  void analyze_import(ASTBase *_p) {
    auto p = ast_must_cast<Import>(_p);

    str file = p->get_filename();
    auto imported = Compiler::resolve_import(_cs->_filename, file);
    if (imported.empty()) {
      report_error(p, "Cannot import: " + file);
    }

    /// it might be already parsed
    vector<FunctionDecl *> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
    if (imported_functions.empty()) {
      Compiler::ParseFile(imported[0]);
      imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
    }

    /// import functions
    p->set_imported_funcs(imported_functions);
    for (FunctionDecl *f: imported_functions) {
      _cs->add_function(f);
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

    auto void_type = ASTType::Create(_cs, Ty::VOID);
    switch (p->get_intrinsic_type()) {
      case IntrinsicType::STACK_TRACE:
      case IntrinsicType::ABORT:
      case IntrinsicType::NOOP: {
        p->set_type(void_type);
        break;
      }
      case IntrinsicType::LINENO: {
        auto sub = IntegerLiteral::Create(_p->get_line(), true);

        // TODO: make a "copy_source_location()" function
        sub->_start_index = p->_start_index;
        sub->_end_index = p->_end_index;
        sub->set_token(p->get_token());

        auto type = ASTType::Create(_cs, TY_OR3(Ty::INT, Ty::UNSIGNED, Ty::BIT32));
        sub->set_type(type);
        p->set_type(type);
        p->set_sub(sub);
        break;
      }
      case IntrinsicType::FILENAME: {
        auto sub = StringLiteral::Create(_cs->_filename);

        sub->_start_index = p->_start_index;
        sub->_end_index = p->_end_index;
        sub->set_token(p->get_token());

        auto type = ASTType::Create(_cs, Ty::STRING);
        sub->set_type(type);
        p->set_type(type);
        p->set_sub(sub);
        break;
      }
      case IntrinsicType::GET_DECL: {
        // FIXME:
        p->set_type(ASTType::Create(_cs, Ty::STRING));
        if (c->get_node_type() != ASTNodeType::STRING_LITERAL) {
          report_error(c, "Expect a string argument");
        }
        // TODO: set p->_value to the source code of p
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
        std::cout << fmt::format("Message ({}): {}\n", _h.get_source_location(p), msg);
        break;
      }
      default:
        report_error(p, "Unknown intrinsic");
    }
  }

  void analyze_string_literal(ASTBase *_p) {
    auto p = ast_must_cast<StringLiteral>(_p);
    p->set_value(p->get_token_str());
    p->set_type(ASTType::Create(_cs, Ty::STRING));
  }

  void analyze_char_literal(ASTBase *_p) {
    auto p = ast_must_cast<CharLiteral>(_p);

    p->set_type(ASTType::Create(_cs, Ty::CHAR, {}));
    p->set_value(static_cast<uint8_t>(p->get_token_str()[0]));
  }

  void analyze_integer_literal(ASTBase *_p) {
    auto p = ast_must_cast<IntegerLiteral>(_p);
    auto tyty = Ty::INT;
    if (p->get_token()->is_unsigned) {
      tyty = TY_OR(tyty, Ty::UNSIGNED);
    }
    p->set_type(ASTType::Create(_cs, tyty));
  }

  void analyze_float_literal(ASTBase *_p) {
    auto p = ast_must_cast<FloatLiteral>(_p);
    p->set_type(ASTType::Create(_cs, Ty::FLOAT));
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

    ASTType *ty = ASTType::Create(_cs, Ty::ARRAY, sub_tys);
    ty->_array_size = elements.size();
    p->set_type(ty);
  }

  void analyze_member_access(MemberAccess *p) {
    Expr *lhs = p->get_lhs();
    analyze(lhs);
    Expr *rhs = p->get_rhs();

    if (p->_access_type == MemberAccess::MemberAccessDeref) { /// pointer dereference
      analyze(rhs);
      auto ty = lhs->get_type();
      TAN_ASSERT(ty->_is_ptr);
      ty = copy_ty(_h.get_contained_ty(ty));
      ty->_is_lvalue = true;
      p->set_type(ty);
    } else if (p->_access_type == MemberAccess::MemberAccessBracket) {
      analyze(rhs);
      auto ty = lhs->get_type();
      if (!ty->_is_ptr) {
        report_error(p, "Expect a pointer or an array");
      }

      ty = copy_ty(_h.get_contained_ty(ty));
      ty->_is_lvalue = true;

      p->set_type(ty);
      if (rhs->get_node_type() == ASTNodeType::INTEGER_LITERAL) {
        auto size_node = ast_must_cast<IntegerLiteral>(rhs);
        auto size = size_node->get_value(); // underflow is ok
        if (rhs->get_type()->_is_array && size >= lhs->get_type()->_array_size) {
          report_error(p,
              fmt::format("Index {} out of bound, the array size is {}",
                  std::to_string(size),
                  std::to_string(lhs->get_type()->_array_size)));
        }
      }
    } else if (rhs->get_node_type() == ASTNodeType::ID) { /// member variable or enum
      analyze(rhs);
      if (lhs->get_type()->_is_enum) {
        // TODO: Member access of enums
        TAN_ASSERT(false);
      } else {
        p->_access_type = MemberAccess::MemberAccessMemberVariable;
        if (!lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
          report_error(p, "Invalid left-hand operand");
        }

        str m_name = ast_must_cast<Identifier>(rhs)->get_name();
        ASTType *struct_ast = nullptr;
        /// auto dereference pointers
        if (lhs->get_type()->_is_ptr) {
          struct_ast = _h.get_contained_ty(lhs->get_type());
        } else {
          struct_ast = lhs->get_type();
        }
        if (struct_ast->_tyty != Ty::STRUCT) {
          report_error(lhs, "Expect a struct type");
        }

        StructDecl *struct_decl = ast_must_cast<StructDecl>(_cs->get_type_decl(struct_ast->_type_name));
        p->_access_idx = struct_decl->get_struct_member_index(m_name);
        auto ty = copy_ty(struct_decl->get_struct_member_ty(p->_access_idx));
        ty->_is_lvalue = true;
        p->set_type(ty);
      }
    } else if (p->_access_type == MemberAccess::MemberAccessMemberFunction) { /// method call
      if (!lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
        report_error(p, "Method calls require left-hand operand to be an lvalue or a pointer");
      }
      auto func_call = ast_cast<FunctionCall>(rhs);
      if (!func_call) {
        report_error(rhs, "Expect a function call");
      }

      /// get address of the struct instance
      if (lhs->get_type()->_is_lvalue && !lhs->get_type()->_is_ptr) {
        Expr *tmp = UnaryOperator::Create(UnaryOpKind::ADDRESS_OF, lhs);
        tmp->_start_index = lhs->_start_index;
        tmp->_end_index = lhs->_end_index;
        tmp->set_token(lhs->get_token());
        analyze(tmp);

        func_call->_args.push_back(tmp);
      } else {
        func_call->_args.push_back(lhs);
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
    _cs->add_type_decl(struct_name, p);

    /// check if struct name is in conflicts of variable/function names
    /// or if there's a forward declaration
    ASTType *prev = _cs->get_type_decl(struct_name)->get_type();
    if (prev) {
      if (prev->get_node_type() != ASTNodeType::STRUCT_DECL) { /// fwd decl
        ty = std::move(prev);
      } else { /// conflict
        report_error(p, "Cannot redeclare type as a struct");
      }
    } else { /// no fwd decl
      ty = ASTType::Create();
      ty->_tyty = Ty::STRUCT;
      _cs->add_type_decl(struct_name, p); /// add self to current scope
    }
    ty->_type_name = struct_name;

    /// resolve member names and types
    auto member_decls = p->get_member_decls();  // size is 0 if no struct body
    size_t n = member_decls.size();
    ty->_sub_types.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      Expr *m = member_decls[i];
      analyze(m);

      if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
        /// fill members
        str name = ast_must_cast<VarDecl>(m)->get_name();
        p->set_member_index(name, i);
        ty->_sub_types.push_back(m->get_type());
      }
        /// member variable with an initial value
      else if (m->get_node_type() == ASTNodeType::ASSIGN) {
        auto bm = ast_must_cast<Assignment>(m);
        // TODO: store default value in the type or the declaration?
        auto init_val = bm->get_rhs();

        if (bm->get_lhs()->get_node_type() != ASTNodeType::VAR_DECL) {
          report_error(bm, "Expect a member variable declaration");
        }
        auto decl = ast_must_cast<VarDecl>(bm->get_lhs());

        /// TODO: support any compile-time known initial values
        //    if (!is_ast_type_in(init_val->get_node_type(), TypeSystem::LiteralTypes)) {
        //      report_error(p, "Invalid initial value of the member variable");
        //    }

        /// fill members
        ty->_sub_types.push_back(decl->get_type());
        p->set_member_index(decl->get_name(), i);
      } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) {
        auto f = ast_must_cast<FunctionDecl>(m);

        /// fill members
        ty->_sub_types.push_back(f->get_type());
        p->set_member_index(f->get_name(), i);
      } else {
        report_error(p, "Invalid struct member");
      }
    }
    resolve_ty(ty);
    p->set_type(ty);
  }

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

using namespace tanlang;

void Analyzer::analyze(ASTBase *p) { _analyzer_impl->analyze(p); }

Analyzer::Analyzer(CompilerSession *cs) { _analyzer_impl = new AnalyzerImpl(cs); }

Analyzer::~Analyzer() { if (_analyzer_impl) { delete _analyzer_impl; }}
