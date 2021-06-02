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
      auto sub = IntegerLiteral::Create(_p->get_line(), true);

      // TODO: make a "copy_source_location()" function
      sub->_start_index = p->_start_index;
      sub->_end_index = p->_end_index;
      sub->set_token(p->get_token());

      auto type = ASTType::Create(_cs, TY_OR3(Ty::INT, Ty::UNSIGNED, Ty::BIT32));
      sub->set_type(type);
      p->set_type(type);
      p->set_sub(sub);
      break;
    }
    case IntrinsicType::FILENAME: {
      auto sub = StringLiteral::Create(_cs->_filename);

      sub->_start_index = p->_start_index;
      sub->_end_index = p->_end_index;
      sub->set_token(p->get_token());

      auto type = ASTType::Create(_cs, Ty::STRING);
      sub->set_type(type);
      p->set_type(type);
      p->set_sub(sub);
      break;
    }
    case IntrinsicType::GET_DECL: {
      // FIXME:
      p->set_type(ASTType::Create(_cs, Ty::STRING));
      if (c->get_node_type() != ASTNodeType::STRING_LITERAL) {
        report_error(c, "Expect a string argument");
      }
      // TODO: set p->_value to the source code of p
      break;
    }
    case IntrinsicType::COMP_PRINT: {
      p->set_type(void_type);

      auto func_call = ast_must_cast<FunctionCall>(c);
      auto args = func_call->_args;

      if (args.size() != 1 || args[0]->get_node_type() != ASTNodeType::STRING_LITERAL) {
        report_error(p, "Invalid call to compprint, one argument with type 'str' required");
      }
      str msg = ast_must_cast<StringLiteral>(args[0])->get_value();
      std::cout << fmt::format("Message ({}): {}\n", _h.get_source_location(p), msg);
      break;
    }
    default:
      report_error(p, "Unknown intrinsic");
  }
}
