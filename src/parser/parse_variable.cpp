#include "base.h"
#include "src/ast/factory.h"
#include "src/ast/ast_ty.h"
#include "src/ast/parsable_ast_node.h"
#include "src/parser/parser_impl.h"

using namespace tanlang;

size_t ParserImpl::parse_var_decl(const ParsableASTNodePtr &p) {
  ++p->_end_index; /// skip 'var'
  return parse_arg_decl(p);
}

size_t ParserImpl::parse_arg_decl(const ParsableASTNodePtr &p) {
  /// var name
  auto name_token = at(p->_end_index);
  p->set_data(name_token->value);
  ++p->_end_index;

  if (at(p->_end_index)->value == ":") {
    ++p->_end_index;
    /// type
    ASTTyPtr ty = ast_create_ty(_cs);
    ty->set_token(at(p->_end_index));
    ty->_end_index = ty->_start_index = p->_end_index;
    ty->_is_lvalue = true;
    p->_end_index = parse_node(ty);
    ast_cast<ASTNode>(p)->_ty = ty;
  } else { ast_cast<ASTNode>(p)->_ty = nullptr; }

  return p->_end_index;
}
