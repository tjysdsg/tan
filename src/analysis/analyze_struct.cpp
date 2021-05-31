#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_struct.h"
#include "src/ast/decl.h"
#include "src/ast/expr.h"
#include "src/common.h"
#include "compiler_session.h"

using namespace tanlang;

void AnalyzerImpl::analyze_struct_decl(const ASTBasePtr &_p) {
  auto p = ast_must_cast<StructDecl>(_p);

  str struct_name = p->get_name();
  ASTStructPtr ty = make_ptr<ASTStruct>();
  ty->_tyty = Ty::STRUCT;

  // TODO: Check if struct name is in conflicts of variable/function names
  _cs->set_type(struct_name, ty); /// add self to current scope

  /// resolve member names and types
  auto member_decls = p->get_member_decls();
  size_t n = member_decls.size();
  ty->_member_names.reserve(n);
  ty->_member_indices.reserve(n);
  ty->_sub_types.reserve(n);
  ty->_initial_values.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    DeclPtr m = member_decls[i];
    if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
      ty->_sub_types.push_back(m->get_type());
    }
      /// member variable with an initial value
    else if (m->get_node_type() == ASTNodeType::BOP
        && ast_must_cast<BinaryOperator>(m)->get_op() == BinaryOpKind::ASSIGN) {
      auto bm = ast_must_cast<BinaryOperator>(m);
      auto init_val = bm->get_rhs();

      m = ast_cast<Decl>(bm->get_lhs());
      if (!m) {
        report_error(bm, "Expect a declaration");
      }

      /// TODO: support any coompile-time known initial values
      if (!is_ast_type_in(init_val->get_node_type(), TypeSystem::LiteralTypes)) {
        report_error(p, "Invalid initial value of the member variable");
      }

      /// fill ASTStruct
      ty->_member_names.push_back(m->get_name());
      ty->_member_indices[m->get_name()] = i;
      ty->_initial_values.push_back(init_val);
      ty->_sub_types.push_back(init_val->get_type());
    } else {
      report_error(p, "Invalid struct member");
    }
  }
  resolve_ty(ty);
  p->set_type(ty);
}
