#include "analyzer_impl.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"
#include "src/ast/expr.h"
#include "src/analysis/type_system.h"
#include "src/ast/intrinsic.h"
#include <fmt/core.h>
#include <iostream>

using namespace tanlang;

void AnalyzerImpl::analyze_intrinsic(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Intrinsic>(_p);
  auto c = p->get_sub();

  /// name
  str name;
  switch (c->get_node_type()) {
    case ASTNodeType::FUNC_CALL:
      name = ast_must_cast<FunctionCall>(c)->get_name();
      break;
    case ASTNodeType::ID:
      name = ast_must_cast<Identifier>(c)->get_name();
      break;
    default:
      TAN_ASSERT(false);
      break;
  }

  /// search for the intrinsic type
  auto q = Intrinsic::intrinsics.find(name);
  if (q == Intrinsic::intrinsics.end()) {
    report_error(p, "Invalid intrinsic");
  }
  p->set_intrinsic_type(q->second);

  auto void_type = ASTType::Create(_cs, Ty::VOID);
  switch (p->get_intrinsic_type()) {
    case IntrinsicType::STACK_TRACE:
    case IntrinsicType::ABORT:
    case IntrinsicType::NOOP: {
      p->set_type(void_type);
      break;
    }
    case IntrinsicType::LINENO: {
      p->set_sub(IntegerLiteral::Create(_p->get_line(), true));
      break;
    }
    case IntrinsicType::FILENAME: {
      p->set_sub(StringLiteral::Create(_cs->_filename));
      break;
    }
    case IntrinsicType::GET_DECL: {
      p->set_type(ASTType::Create(_cs, Ty::STRING));
      if (c->get_node_type() != ASTNodeType::STRING_LITERAL) {
        report_error(c, "Expect a string argument");
      }
      // TODO: set p->_value to the source code of p
      break;
    }
    case IntrinsicType::COMP_PRINT: {
      p->set_type(void_type);
      if (c->get_node_type() != ASTNodeType::STRING_LITERAL) {
        report_error(p, "Invalid call to compprint, one argument with type 'str' required");
      }
      std::cout << fmt::format("Message ({}): {}\n", _h.get_source_location(p), name);
      break;
    }
    default:
      report_error(p, "Unknown intrinsic");
  }
}
