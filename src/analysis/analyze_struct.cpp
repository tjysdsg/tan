#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_struct.h"
#include "src/ast/decl.h"
#include "src/common.h"
#include "compiler_session.h"

using namespace tanlang;

void AnalyzerImpl::analyze_struct_decl(const ASTBasePtr &_p) {
  auto p = ast_must_cast<StructDecl>(_p);

  ASTStructPtr ty = nullptr;

  /// check if struct name is in conflicts of variable/function names
  /// or if there's a forward declaration
  str struct_name = p->get_name();
  auto prev = _cs->get_type(struct_name);
  if (prev) {
    if (prev->get_node_type() != ASTNodeType::STRUCT_DECL) { /// fwd decl
      ty = ast_must_cast<ASTStruct>(std::move(prev));
    } else { /// conflict
      report_error(p, "Cannot redeclare type as a struct");
    }
  } else { /// no fwd decl
    ty = make_ptr<ASTStruct>();
    ty->_tyty = Ty::STRUCT;
    _cs->set_type(struct_name, ty); /// add self to current scope
  }

  /// resolve member names and types
  auto member_decls = p->get_member_decls();  // size is 0 if no struct body
  size_t n = member_decls.size();
  ty->_member_names.reserve(n);
  ty->_member_indices.reserve(n);
  ty->_sub_types.reserve(n);
  ty->_initial_values.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    ExprPtr m = member_decls[i];
    analyze(m);

    if (m->get_node_type() == ASTNodeType::VAR_DECL) { /// member variable without initial value
      /// fill members
      str name = ast_must_cast<VarDecl>(m)->get_name();
      ty->_sub_types.push_back(m->get_type());
      ty->_member_names.push_back(name);
      ty->_member_indices[name] = i;
    }
      /// member variable with an initial value
    else if (m->get_node_type() == ASTNodeType::ASSIGN) {
      auto bm = ast_must_cast<Assignment>(m);
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
      auto name = decl->get_name();
      auto type = decl->get_type();
      ty->_sub_types.push_back(type);
      ty->_member_names.push_back(name);
      ty->_member_indices[name] = i;
      ty->_initial_values[name] = init_val;
    } else if (m->get_node_type() == ASTNodeType::FUNC_DECL) {
      auto f = ast_must_cast<FunctionDecl>(m);

      /// fill members
      auto name = f->get_name();
      auto type = f->get_type();
      ty->_sub_types.push_back(type);
      ty->_member_names.push_back(name);
      ty->_member_indices[name] = i;
    } else {
      report_error(p, "Invalid struct member");
    }
  }
  resolve_ty(ty);
  p->set_type(ty);
}
