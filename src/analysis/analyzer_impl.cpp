#include "analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"
#include "src/ast/factory.h"
#include "src/ast/ast_base.h"
#include "src/analysis/type_system.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

ASTTypepePtr AnalyzerImpl::copy_ty(const ASTTypePtr &p) const {
  return make_ptr<ASTType>(*p);
}

void AnalyzerImpl::analyze(const ASTBasePtr &p) {
  p->set_scope(_cs->get_current_scope());
  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  /// children will not be automatically parsed for FUNC_DECL or ASSIGN
  vector<ASTNodeType> tmp = {ASTNodeType::FUNC_DECL, ASTNodeType::ASSIGN};
  if (!std::any_of(tmp.begin(), tmp.end(), [p](ASTNodeType i) { return i == p->get_node_type(); })) {
    for (auto &sub: p->get_children()) {
      analyze(sub);
    }
  }

  switch (p->get_node_type()) {
    /////////////////////////// binary ops ///////////////////////////////////
    // TODO: create a new ASTNodeType for unary plus and minus
    case ASTNodeType::SUM:
    case ASTNodeType::SUBTRACT: {
      /// unary plus/minus
      if (p->get_children_size() == 1) {
        np->_type = copy_ty(_h.get_ty(p->get_child_at(0)));
        np->_type->_is_lvalue = false;
        break;
      }
    }
      // fallthrough
    case ASTNodeType::MULTIPLY:
    case ASTNodeType::DIVIDE:
    case ASTNodeType::MOD: {
      int i = TypeSystem::CanImplicitCast(_cs, _h.get_ty(p->get_child_at(0)), _h.get_ty(p->get_child_at(1)));
      if (i == -1) {
        report_error(p, "Cannot perform implicit type conversion");
      }

      size_t dominant_idx = static_cast<size_t>(i);
      np->_dominant_idx = dominant_idx;
      np->_type = copy_ty(_h.get_ty(p->get_child_at(dominant_idx)));
      np->_type->_is_lvalue = false;
      break;
    }
    case ASTNodeType::GT:
    case ASTNodeType::GE:
    case ASTNodeType::LT:
    case ASTNodeType::LE:
    case ASTNodeType::EQ:
    case ASTNodeType::NE:
      np->_type = create_ty(_cs, Ty::BOOL);
      break;
    case ASTNodeType::ASSIGN:
      analyze_assignment(p);
      break;
    case ASTNodeType::CAST: {
      np->_type = copy_ty(_h.get_ty(p->get_child_at(1)));
      np->_type->_is_lvalue = _h.get_ty(p->get_child_at(0))->_is_lvalue;
      // FIXME: check if can explicit cast
      // if (TypeSystem::CanImplicitCast(_cs, np->_type, _h.get_ty(p->get_child_at(0))) != 0) {
      //   report_error(p, "Cannot perform implicit type conversion");
      // }
      break;
    }
    case ASTNodeType::MEMBER_ACCESS:
      analyze_member_access(p);
      break;
      /////////////////////////// unary ops ////////////////////////////////////
    case ASTNodeType::RET:
      // TODO: check if return type can be implicitly cast to function return type
      break;
    case ASTNodeType::LNOT:
      np->_type = create_ty(_cs, Ty::BOOL);
      break;
    case ASTNodeType::BNOT:
      np->_type = copy_ty(_h.get_ty(p->get_child_at(0)));
      break;
    case ASTNodeType::ADDRESS_OF: {
      np->_type = copy_ty(_h.get_ty(p->get_child_at(0)));
      if (!np->_type) {
        report_error(p, "Invalid operand");
      }
      np->_type = _h.get_ptr_to(np->_type);
      break;
    }
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
