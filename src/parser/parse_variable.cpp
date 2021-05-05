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

size_t ParserImpl::parse_var_decl(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip 'var'
  return parse_arg_decl(p);
}

size_t ParserImpl::parse_arg_decl(const ParsableASTNodePtr &p) {
  /// var name
  auto name_token = at(p->_end_index);
  p->_name = name_token->value;
  ++p->_end_index;
  if (at(p->_end_index)->value == ":") {
    ++p->_end_index;
    /// type
    auto ty = ast_create_ty(_cs);
    ty->set_token(at(p->_end_index));
    ty->_end_index = ty->_start_index = p->_end_index;
    ty->_is_lvalue = true;
    p->_end_index = parse_node(ty);
    // FIXME: p->_ty = ty;
  } else { p->_ty = nullptr; }

  return p->_end_index;
}
