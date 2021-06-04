#include "analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_base.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

ASTType *AnalyzerImpl::copy_ty(ASTType *p) const { return new ASTType(*p); }

void AnalyzerImpl::analyze(ASTBase *p) {
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
    default:
      TAN_ASSERT(false);
      break;
  }
}

void AnalyzerImpl::resolve_ty(ASTType *p) const {
  TypeSystem::ResolveTy(_cs, p);
}

AnalyzerImpl::AnalyzerImpl(CompilerSession *cs) : _cs(cs), _h(ASTHelper(cs)) {}

void AnalyzerImpl::report_error(ASTBase *p, const str &message) {
  ::report_error(_cs->_filename, p->get_token(), message);
}

void AnalyzerImpl::analyze_uop(ASTBase *_p) {
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

void AnalyzerImpl::analyze_id(ASTBase *_p) {
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

void AnalyzerImpl::analyze_parenthesis(ASTBase *_p) {
  auto p = ast_must_cast<Parenthesis>(_p);

  analyze(p->get_sub());

  p->set_type(copy_ty(p->get_sub()->get_type()));
}

void AnalyzerImpl::analyze_if(ASTBase *_p) {
  auto p = ast_must_cast<If>(_p);

  auto cond = p->get_predicate();
  analyze(cond);

  analyze(p->get_then());
  if (p->get_else()) { analyze(p->get_else()); }

  if (0 != TypeSystem::CanImplicitCast(_cs, ASTType::Create(_cs, Ty::BOOL), cond->get_type())) {
    report_error(p, "Cannot convert type to bool");
  }
}

void AnalyzerImpl::analyze_var_decl(ASTBase *_p) {
  auto p = ast_must_cast<VarDecl>(_p);
  ASTType *ty = p->get_type();

  if (ty) { /// analyze type if specified
    ty->_is_lvalue = true;
    analyze(ty);
  }

  _cs->add(p->get_name(), p);
}

void AnalyzerImpl::analyze_arg_decl(ASTBase *_p) {
  auto p = ast_must_cast<ArgDecl>(_p);
  ASTType *ty = p->get_type();
  ty->_is_lvalue = true;
  analyze(ty);
  _cs->add(p->get_name(), p);
}

void AnalyzerImpl::analyze_ret(ASTBase *_p) {
  // TODO: check if return type can be implicitly cast to function return type
  auto p = ast_must_cast<Return>(_p);
  analyze(p->get_rhs());
}

void AnalyzerImpl::analyze_stmt(ASTBase *_p) {
  auto p = ast_must_cast<CompoundStmt>(_p);
  for (const auto &c: p->get_children()) {
    analyze(c);
  }
}
