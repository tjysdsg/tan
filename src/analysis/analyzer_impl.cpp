#include "analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_base.h"
#include "src/ast/expr.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

ASTTypePtr AnalyzerImpl::copy_ty(const ASTTypePtr &p) const {
  return make_ptr<ASTType>(*p);
}

void AnalyzerImpl::analyze(const ASTBasePtr &p) {
  p->set_scope(_cs->get_current_scope());

  /// children will not be automatically parsed for FUNC_DECL or ASSIGN
  vector<ASTNodeType> tmp = {ASTNodeType::FUNC_DECL, ASTNodeType::ASSIGN};
  if (!std::any_of(tmp.begin(), tmp.end(), [p](ASTNodeType i) { return i == p->get_node_type(); })) {
    for (auto &sub: p->get_children()) {
      analyze(sub);
    }
  }

  switch (p->get_node_type()) {
    /////////////////////////// binary ops ///////////////////////////////////
    case ASTNodeType::BOP:
      analyze_bop(p);
    case ASTNodeType::UOP:
      analyze_uop(p);
    case ASTNodeType::SUM:
    case ASTNodeType::SUBTRACT: {
      /// unary plus/minus
      if (p->get_children_size() == 1) {
        np->_type = copy_ty(_h.get_ty(p->get_child_at(0)));
        np->_type->_is_lvalue = false;
        break;
      }
    }
      /////////////////////////// unary ops ////////////////////////////////////
    case ASTNodeType::RET:
      // TODO: check if return type can be implicitly cast to function return type
      break;
    case ASTNodeType::ID: {
      auto referred = _h.get_id_referred(ast_must_cast<ASTNode>(p));
      if (!referred) {
        report_error(p, "Unknown identifier");
      }
      p->append_child(referred);
      np->_type = copy_ty(referred->_type);
      np->_type->_is_lvalue = true;
      break;
    }
      //////////////////////// literals ///////////////////////////////////////
    case ASTNodeType::STRING_LITERAL:
      analyze_string_literal(p);
      break;
    case ASTNodeType::CHAR_LITERAL:
      analyze_char_literal(p);
      break;
    case ASTNodeType::NUM_LITERAL:
      analyze_num_literal(p);
      break;
    case ASTNodeType::ARRAY_LITERAL:
      analyze_array_literal(p);
      break;
      ////////////////////////// keywords ///////////////////////////
    case ASTNodeType::IF: {
      auto cond = p->get_child_at(0);
      if (0 != TypeSystem::CanImplicitCast(_cs, create_ty(_cs, Ty::BOOL), _h.get_ty(cond))) {
        report_error(p, "Cannot convert type to bool");
      }
      break;
    }
      // TODO: cs->set_current_loop(pl) // case ASTNodeType::LOOP:
      // TODO: cs->get_current_loop() // case ASTNodeType::BREAK (or CONTINUE):
      ////////////////////////// others ///////////////////////////
    case ASTNodeType::INTRINSIC: {
      analyze_intrinsic(p);
      break;
    }
    case ASTNodeType::IMPORT:
      analyze_import(p);
      break;
    case ASTNodeType::PARENTHESIS:
      np->_type = copy_ty(_h.get_ty(p->get_child_at(0)));
      break;
    case ASTNodeType::FUNC_CALL:
      analyze_func_call(p);
      break;
    case ASTNodeType::TY: {
      ASTTypePtr pt = ast_must_cast<ASTType>(p);
      resolve_ty(pt);
      break;
    }
      ////////////////////////// declarations ///////////////////////////
    case ASTNodeType::ENUM_DECL: {
      // TODO: Analysis of enum types and values
      break;
    }
    case ASTNodeType::FUNC_DECL:
      analyze_func_decl(p);
      break;
    case ASTNodeType::ARG_DECL:
    case ASTNodeType::VAR_DECL: {
      ASTTypePtr ty = _h.get_ty(p);
      ty->_is_lvalue = true;
      resolve_ty(ty);
      _cs->add(p->get_data<str>(), ast_must_cast<ASTNode>(p));
      break;
    }
    case ASTNodeType::STRUCT_DECL:
      analyze_struct(p);
      break;
      /////////////////////// trivially analysed /////////////////////////////
    default:
      break;
  }
}

void AnalyzerImpl::resolve_ty(const ASTTypePtr &p) const {
  TypeSystem::ResolveTy(_cs, p);
}

AnalyzerImpl::AnalyzerImpl(CompilerSession *cs) : _cs(cs), _h(ASTHelper(cs)) {
}

void AnalyzerImpl::report_error(const ASTBasePtr &p, const str &message) {
  ::report_error(_cs->_filename, p->get_token(), message);
}

void AnalyzerImpl::analyze_uop(const ASTBasePtr &_p) {
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
    default:
      TAN_ASSERT(false);
      break;
  }
}
