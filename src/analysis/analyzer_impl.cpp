#include "analyzer_impl.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "src/analysis/analysis.h"
#include "src/ast/factory.h"
#include "src/ast/parsable_ast_node.h"
#include "src/analysis/type_system.h"
#include "src/common.h"
#include "token.h"

using namespace tanlang;

ASTNodePtr AnalyzerImpl::try_convert_to_ast_node(const ParsableASTNodePtr &p) const {
  ASTNodePtr ret = nullptr;
  if (p->get_node_type() != ASTType::TY) {
    ret = ast_must_cast<ASTNode>(p);
  }
  return ret;
}

ASTTyPtr AnalyzerImpl::get_ty(const ParsableASTNodePtr &p) const {
  ASTNodePtr np = ast_cast<ASTNode>(p);
  TAN_ASSERT(np);
  return np->_ty;
}

void AnalyzerImpl::analyze(ParsableASTNodePtr& p) {
  // TODO: p->_scope = _cs->get_current_scope();
  // TODO: update _cs->_current_token
  ASTNodePtr np = try_convert_to_ast_node(p);

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
        np->_ty = get_ty(p->get_child_at(0));
        break;
      }
    }
      // fallthrough
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD: {
      int i = TypeSystem::CanImplicitCast(_cs, get_ty(p->get_child_at(0)), get_ty(p->get_child_at(1)));
      if (i == -1) {
        report_error(_cs, p, "Cannot perform implicit type conversion");
      }

      size_t dominant_idx = static_cast<size_t>(i);
      np->_dominant_idx = dominant_idx;
      np->_ty = get_ty(p->get_child_at(dominant_idx));
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
      np->_ty = get_ty(p->get_child_at(0));
      if (TypeSystem::CanImplicitCast(_cs, np->_ty, get_ty(p->get_child_at(1))) != 0) {
        report_error(_cs, p, "Cannot perform implicit type conversion");
      }
      break;
    }
    case ASTType::CAST: {
      np->_ty = make_ptr<ASTTy>(*get_ty(p->get_child_at(1)));
      np->_ty->_is_lvalue = get_ty(p->get_child_at(0))->_is_lvalue;
      if (TypeSystem::CanImplicitCast(_cs, np->_ty, get_ty(p->get_child_at(0))) != 0) {
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
      np->_ty = get_ty(p->get_child_at(0));
      break;
    case ASTType::ADDRESS_OF: {
      if (!(np->_ty = get_ty(p->get_child_at(0)))) { report_error(_cs, p, "Invalid operand"); }
      np->_ty = get_ptr_to(cs, np->_ty);
      break;
    }
    case ASTType::ID: {
      auto referred = get_id_referred(cs, p);
      p->append_child(referred);
      np->_ty = referred->_ty;
      break;
    }
      //////////////////////// literals ///////////////////////////////////////
    case ASTType::CHAR_LITERAL: {
      np->_ty = create_ty(cs, Ty::CHAR, {});
      np->_value = static_cast<uint64_t>(p->get_token()->value[0]);
      np->_ty->_default_value = std::get<uint64_t>(p->_value);
      break;
    }
    case ASTType::NUM_LITERAL: {
      if (p->get_token()->type == TokenType::INT) {
        auto tyty = Ty::INT;
        if (p->get_token()->is_unsigned) { tyty = TY_OR(tyty, Ty::UNSIGNED); }
        np->_ty = create_ty(cs, tyty);
      } else if (p->get_token()->type == TokenType::FLOAT) {
        np->_ty = create_ty(cs, Ty::FLOAT);
      }
      break;
    }
    case ASTType::ARRAY_LITERAL: {
      vector<ASTNodePtr> sub_tys{};
      sub_tys.reserve(p->get_children_size());
      std::for_each(p->_children.begin(), p->_children.end(), [&sub_tys](const ASTNodePtr &e) {
        sub_tys.push_back(e->_ty);
      });
      np->_ty = create_ty(cs, Ty::ARRAY, sub_tys);
      break;
    }
      ////////////////////////// keywords ///////////////////////////
    case ASTType::IF: {
      auto cond = p->get_child_at(0);
      if (0 != TypeSystem::CanImplicitCast(cs, create_ty(cs, Ty::BOOL), cond->_ty)) {
        report_error(cs, p, "Cannot convert type to bool");
      }
      break;
    }
      // TODO: cs->set_current_loop(pl) // case ASTType::LOOP:
      // TODO: cs->get_current_loop() // case ASTType::BREAK (or CONTINUE):
      ////////////////////////// others ///////////////////////////
    case ASTType::INTRINSIC: {
      analyze_intrinsic(cs, p);
      break;
    }
    case ASTType::IMPORT: {
      // TODO: determine whether to use class field or child ASTNode to store imported filename
      // auto rhs = p->get_child_at(0);
      // str file = std::get<str>(rhs->_value);
      str file = p->_name;
      auto imported = Compiler::resolve_import(cs->_filename, file);
      if (imported.empty()) { report_error(cs, p, "Cannot import: " + file); }

      /// it might be already parsed
      vector<ASTFunctionPtr> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
      if (imported_functions.empty()) {
        Compiler::ParseFile(imported[0]);
        imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
      }
      for (auto &f: imported_functions) {
        cs->add_function(f);
        p->_children.push_back(f);
      }
      break;
    }
    case ASTType::PARENTHESIS:
      np->_ty = p->get_child_at(0)->_ty;
      break;
    case ASTType::FUNC_CALL: {
      std::vector<ASTNodePtr> args(p->_children.begin(), p->_children.end());
      p->_children.clear();
      p->_children.push_back(ASTFunction::GetCallee(cs, p->_name, args));
      np->_ty = p->get_child_at(0)->_ty;
      break;
    }
    case ASTType::TY: {
      ASTTyPtr pt = ast_must_cast<ASTTy>(p);
      resolve_ty(cs, pt);
      break;
    }
      ////////////////////////// declarations ///////////////////////////
    case ASTType::ENUM_DECL: {
      // TODO: Analysis of enum types and values
      break;
    }
    case ASTType::FUNC_DECL: {
      /// add to function table
      if (p->_is_public || p->_is_external) { CompilerSession::AddPublicFunction(cs->_filename, p); }
      /// ...and to the internal function table
      cs->add_function(p);

      // TODO: function type
      //  auto ret_ty = ast_create_ty(_cs);
      //  ret_ty->set_token(at(p->_end_index));
      //  ret_ty->_end_index = ret_ty->_start_index = p->_end_index;
      //  p->_end_index = parse_ty(ret_ty); /// return type
      //  p->get_child_at(0) = ret_ty;

      /// add args to scope if function body exists
      size_t n = p->get_children_size();
      size_t arg_end = n - 1 - !p->_is_external;
      for (size_t i = 1; i < arg_end; ++i) {
        if (!p->_is_external) { cs->add(p->get_child_at(i)->_name, p->get_child_at(i)); }
      }
      if (!p->_is_external) {
        /// new scope for function body
        auto f_body = p->_children[n - 1];
        if (!p->_is_external) {
          f_body->_scope = cs->push_scope();
        }
      }
      break;
    }
    case ASTType::ARG_DECL:
    case ASTType::VAR_DECL: {
      resolve_ty(cs, p->_ty);
      cs->add(p->_name, p);
      break;
    }
    case ASTType::STRUCT_DECL: {
      str struct_name = p->get_child_at(0)->_name;
      auto ty = ast_create_ty(cs);
      ty->_tyty = Ty::STRUCT;

      auto forward_decl = cs->get(struct_name);
      // TODO: Check if struct name is in conflicts of variable/function names
      if (!forward_decl) {
        cs->add(struct_name, ty); /// add self to current scope
      } else {
        /// replace forward decl with self (even if this is a forward declaration too)
        cs->set(struct_name, ty);
      }

      /// resolve member names and types
      auto members = p->_children;
      size_t n = p->get_children_size();
      ty->_member_names.reserve(n);
      ty->_children.reserve(n);
      for (size_t i = 0; i < n; ++i) {
        ASTNodePtr m = members[i];
        if (members[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
          ty->_children.push_back(m->_ty);
        } else if (members[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
          auto init_val = m->get_child_at(1);
          m = m->get_child_at(0);
          if (!is_ast_type_in(init_val->_type, TypeSystem::LiteralTypes)) {
            report_error(cs, p, "Invalid initial value of the member variable");
          }
          ty->_children.push_back(init_val->_ty); /// init_val->_ty->_default_value is set to the initial value
        } else { report_error(cs, p, "Invalid struct member"); }
        ty->_member_names.push_back(m->_name);
        ty->_member_indices[m->_name] = i;
      }
      resolve_ty(cs, ty);
      np->_ty = ty;
      break;
    }
      /////////////////////// trivially analysed /////////////////////////////
    default:
      break;
  }
}

AnalyzerImpl::AnalyzerImpl(CompilerSession *cs) : _cs(cs) {
}

void AnalyzerImpl::resolve_ty(const ASTTyPtr &p) const {

}


