#include "analyzer_impl.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "src/ast/factory.h"
#include "src/ast/parsable_ast_node.h"
#include "src/analysis/type_system.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

void AnalyzerImpl::analyze(ParsableASTNodePtr &p) {
  // TODO: p->_scope = _cs->get_current_scope();
  // TODO: update _cs->_current_token
  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  if (p->get_node_type() == ASTType::FUNC_DECL) { /// children will not be automatically parsed for function declaration
    for (auto &sub: p->get_children()) {
      analyze(sub);
    }
  }

  switch (p->get_node_type()) {
    /////////////////////////// binary ops ///////////////////////////////////
    // TODO: create a new ASTType for unary plus and minus
    case ASTType::SUM:
    case ASTType::SUBTRACT: {
      /// unary plus/minus
      if (p->get_children_size() == 1) {
        np->_ty = _h.get_ty(p->get_child_at(0));
        break;
      }
    }
      // fallthrough
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD: {
      int i = TypeSystem::CanImplicitCast(_cs, _h.get_ty(p->get_child_at(0)), _h.get_ty(p->get_child_at(1)));
      if (i == -1) {
        report_error(_cs, p, "Cannot perform implicit type conversion");
      }

      size_t dominant_idx = static_cast<size_t>(i);
      np->_dominant_idx = dominant_idx;
      np->_ty = _h.get_ty(p->get_child_at(dominant_idx));
      break;
    }
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
      np->_ty = create_ty(_cs, Ty::BOOL);
      break;
    case ASTType::ASSIGN: {
      np->_ty = _h.get_ty(p->get_child_at(0));
      if (TypeSystem::CanImplicitCast(_cs, np->_ty, _h.get_ty(p->get_child_at(1))) != 0) {
        report_error(_cs, p, "Cannot perform implicit type conversion");
      }
      break;
    }
    case ASTType::CAST: {
      np->_ty = make_ptr<ASTTy>(*_h.get_ty(p->get_child_at(1)));
      np->_ty->_is_lvalue = _h.get_ty(p->get_child_at(0))->_is_lvalue;
      if (TypeSystem::CanImplicitCast(_cs, np->_ty, _h.get_ty(p->get_child_at(0))) != 0) {
        report_error(_cs, p, "Cannot perform implicit type conversion");
      }
      break;
    }
    case ASTType::MEMBER_ACCESS:
      analyze_member_access(p);
      break;
      /////////////////////////// unary ops ////////////////////////////////////
    case ASTType::RET:
      // TODO: check if return type can be implicitly cast to function return type
      break;
    case ASTType::LNOT:
      np->_ty = create_ty(_cs, Ty::BOOL);
      break;
    case ASTType::BNOT:
      np->_ty = _h.get_ty(p->get_child_at(0));
      break;
    case ASTType::ADDRESS_OF: {
      if (!(np->_ty = _h.get_ty(p->get_child_at(0)))) { report_error(_cs, p, "Invalid operand"); }
      np->_ty = _h.get_ptr_to(np->_ty);
      break;
    }
    case ASTType::ID: {
      auto referred = _h.get_id_referred(p);
      p->append_child(referred);
      np->_ty = referred->_ty;
      break;
    }
      //////////////////////// literals ///////////////////////////////////////
    case ASTType::CHAR_LITERAL:
      analyze_char_literal(p);
      break;
    case ASTType::NUM_LITERAL:
      analyze_num_literal(p);
      break;
    case ASTType::ARRAY_LITERAL:
      analyze_array_literal(p);
      break;
      ////////////////////////// keywords ///////////////////////////
    case ASTType::IF: {
      auto cond = p->get_child_at(0);
      if (0 != TypeSystem::CanImplicitCast(_cs, create_ty(_cs, Ty::BOOL), _h.get_ty(cond))) {
        report_error(_cs, p, "Cannot convert type to bool");
      }
      break;
    }
      // TODO: cs->set_current_loop(pl) // case ASTType::LOOP:
      // TODO: cs->get_current_loop() // case ASTType::BREAK (or CONTINUE):
      ////////////////////////// others ///////////////////////////
    case ASTType::INTRINSIC: {
      analyze_intrinsic(p);
      break;
    }
    case ASTType::IMPORT:
      analyze_import(p);
      break;
    case ASTType::PARENTHESIS:
      np->_ty = _h.get_ty(p->get_child_at(0));
      break;
    case ASTType::FUNC_CALL:
      analyze_func_call(p);
      break;
    case ASTType::TY: {
      ASTTyPtr pt = ast_must_cast<ASTTy>(p);
      resolve_ty(pt);
      break;
    }
      ////////////////////////// declarations ///////////////////////////
    case ASTType::ENUM_DECL: {
      // TODO: Analysis of enum types and values
      break;
    }
    case ASTType::FUNC_DECL:
      analyze_func_decl(p);
      break;
    case ASTType::ARG_DECL:
    case ASTType::VAR_DECL: {
      resolve_ty(_h.get_ty(p));
      _cs->set_type(p->get_data<str>(), p);
      break;
    }
    case ASTType::STRUCT_DECL:
      analyze_struct(p);
      break;
      /////////////////////// trivially analysed /////////////////////////////
    default:
      break;
  }
}

void AnalyzerImpl::resolve_ty(const ASTTyPtr &p) const {

}

AnalyzerImpl::AnalyzerImpl(CompilerSession *cs) : _cs(cs), _h(ASTHelper(cs)) {
}

void AnalyzerImpl::report_error(const ParsableASTNodePtr &p, const str &message) {
  ::report_error(_cs->_filename, p->get_token(), message);
}
