#include "base.h"
#include "src/ast/ast_type.h"
#include "src/parser/parser_impl.h"
#include "src/ast/decl.h"

using namespace tanlang;

size_t ParserImpl::parse_var_decl(const ASTBasePtr &_p) {
  ptr<ArgDecl> p = ast_must_cast<ArgDecl>(_p);

  ++p->_end_index; /// skip 'var'

  /// name
  auto name_token = at(p->_end_index);
  p->set_name(name_token->value);
  ++p->_end_index;

  /// type
  if (at(p->_end_index)->value == ":") {
    ++p->_end_index;
    ASTTypePtr ty = ASTType::Create();
    ty->set_token(at(p->_end_index));
    ty->_end_index = ty->_start_index = p->_end_index;
    ty->_is_lvalue = true;
    p->_end_index = parse_node(ty);
    p->set_type(ty);
  }

  return p->_end_index;
}

size_t ParserImpl::parse_arg_decl(const ASTBasePtr &_p) {
  ptr<ArgDecl> p = ast_must_cast<ArgDecl>(_p);

  /// name
  auto name_token = at(p->_end_index);
  p->set_name(name_token->value);
  ++p->_end_index;

  if (at(p->_end_index)->value != ":") {
    error(p->_end_index, "Expect a type being specified");
  }
  ++p->_end_index;

  /// type
  ASTTypePtr ty = ASTType::Create();
  ty->set_token(at(p->_end_index));
  ty->_end_index = ty->_start_index = p->_end_index;
  ty->_is_lvalue = true;
  p->_end_index = parse_node(ty);
  p->set_type(ty);

  return p->_end_index;
}
