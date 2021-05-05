#include "parser.h"
#include "base.h"
#include "compiler_session.h"
#include "src/parser/parser_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_member_access.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "intrinsic.h"
#include "token.h"
#include <memory>
#include <utility>

using namespace tanlang;

size_t ParserImpl::parse_struct_decl(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip "struct"

  /// struct name
  auto id = peek(p->_end_index);
  if (id->_type != ASTType::ID) {
    error(p->_end_index, "Expect struct name");
  }
  p->_name = id->_name;

  /// struct body
  if (at(p->_end_index)->value == "{") {
    auto comp_stmt = next_expression(p->_end_index);
    if (!comp_stmt || comp_stmt->_type != ASTType::STATEMENT) {
      error(comp_stmt->_end_index, "Invalid struct body");
    }
    p->_children = comp_stmt->_children;
  }

  return p->_end_index;
}
