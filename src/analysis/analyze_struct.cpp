#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "compiler_session.h"

using namespace tanlang;

void AnalyzerImpl::analyze_struct(ParsableASTNodePtr &p) {
  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  str struct_name = p->get_child_at(0)->get_data<str>();
  auto ty = ast_create_ty(_cs);
  ty->_tyty = Ty::STRUCT;

  ASTTyPtr forward_decl = _cs->get_type(struct_name);
  // TODO: Check if struct name is in conflicts of variable/function names
  if (!forward_decl) {
    _cs->set_type(struct_name, ty); /// add self to current scope
  } else {
    /// replace forward decl with self (even if this is a forward declaration too)
    _cs->set_type(struct_name, ty);
  }

  /// resolve member names and types
  auto members = p->get_children();
  size_t n = p->get_children_size();
  ty->_member_names.reserve(n);
  ty->get_children().reserve(n);
  for (size_t i = 0; i < n; ++i) {
    ASTNodePtr m = members[i];
    if (members[i]->get_node_type() == ASTType::VAR_DECL) { /// member variable without initial value
      ty->append_child(m->_ty);
    } else if (members[i]->get_node_type() == ASTType::ASSIGN) { /// member variable with an initial value
      auto init_val = m->get_child_at(1);
      m = m->get_child_at(0);
      if (!is_ast_type_in(init_val->get_node_type(), TypeSystem::LiteralTypes)) {
        report_error(_cs, p, "Invalid initial value of the member variable");
      }
      ty->append_child(_h.get_ty(init_val)); /// init_val->_ty->_default_value is set to the initial value
    } else { report_error(_cs, p, "Invalid struct member"); }
    ty->_member_names.push_back(m->get_data<str>());
    ty->_member_indices[m->get_data<str>()] = i;
  }
  resolve_ty(ty);
  np->_ty = ty;
}
