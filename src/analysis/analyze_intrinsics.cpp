#include "analyzer_impl.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "src/ast/factory.h"
#include "src/ast/parsable_ast_node.h"
#include "src/analysis/type_system.h"
#include "intrinsic.h"
#include <iostream>

using namespace tanlang;

void AnalyzerImpl::analyze_intrinsic(ParsableASTNodePtr &p) {
  auto pi = ast_must_cast<Intrinsic>(p);
  TAN_ASSERT(p->get_children_size());

  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  auto c = p->get_child_at<ASTNode>(0);
  TAN_ASSERT(c->_is_named);  // both function call and identifier have a name
  auto void_type = create_ty(_cs, Ty::VOID);
  auto q = Intrinsic::intrinsics.find(c->get_data<str>());
  if (q == Intrinsic::intrinsics.end()) {
    report_error(_cs, p, "Invalid intrinsic");
  }
  pi->_intrinsic_type = q->second;
  switch (pi->_intrinsic_type) {
    case IntrinsicType::STACK_TRACE:
    case IntrinsicType::ABORT:
    case IntrinsicType::NOOP: {
      np->_ty = void_type;
      break;
    }
    case IntrinsicType::LINENO: {
      p->set_child_at(0, ast_create_numeric_literal(_cs, c->get_line()));
      break;
    }
    case IntrinsicType::FILENAME: {
      p->set_child_at(0, ast_create_string_literal(_cs, _cs->_filename));
      break;
    }
    case IntrinsicType::GET_DECL: {
      np->_ty = create_ty(_cs, Ty::STRING);
      TAN_ASSERT(c->get_node_type() == ASTType::STRING_LITERAL);
      // TODO: set p->_value to the source code of p
      break;
    }
    case IntrinsicType::COMP_PRINT: {
      np->_ty = void_type;
      if (c->get_node_type() != ASTType::STRING_LITERAL) {
        report_error(_cs, p, "Invalid call to compprint, one argument with type 'str' required");
      }
      std::cout << "Message (" << _h.get_source_location(p) << "): " << c->get_data<str>() << "\n";
      break;
    }
    default:
      report_error(_cs, p, "Unknown intrinsic");
  }
}
