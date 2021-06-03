#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/decl.h"
#include "src/common.h"
#include "compiler_session.h"

using namespace tanlang;

void AnalyzerImpl::analyze_struct_decl(const ASTBasePtr &_p) {
  auto p = ast_must_cast<StructDecl>(_p);

  ASTTypePtr ty = nullptr;
  str struct_name = p->get_name();
  _cs->add_type_decl(struct_name, p);

  /// check if struct name is in conflicts of variable/function names
  /// or if there's a forward declaration
  ASTTypePtr prev = _cs->get_type_decl(struct_name)->get_type();
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
    ExprPtr m = member_decls[i];
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
