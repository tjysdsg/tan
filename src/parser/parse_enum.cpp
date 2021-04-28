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

size_t ParserImpl::parse_enum_decl(const ASTNodePtr &p) {
  ++p->_end_index; /// skip "enum"
  auto name = peek(p->_end_index);
  if (name->_type != ASTType::ID) {
    error(p->_end_index, "Expect an enum name");
  }

  /// enum body
  if (at(p->_end_index)->value != "{") {
    error(p->_end_index, "Invalid enum declaration");
  }
  ++p->_end_index;
  while (!eof(p->_end_index) && at(p->_end_index)->value != "}") {
    auto e = ast_create_statement(_cs);
    p->_children.push_back(e);
    if (at(p->_end_index)->value == ",") { ++p->_end_index; }
  }
  ++p->_end_index; /// skip '}'

  return p->_end_index;
}
