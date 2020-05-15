#include "src/ast/ast_arg_decl.h"
#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_ty.h"

using namespace tanlang;

ASTArgDecl::ASTArgDecl(Token *token, size_t token_index) : ASTVarDecl(token, token_index) {
  _type = ASTType::ARG_DECL;
}

size_t ASTArgDecl::nud() {
  _end_index = _start_index;
  auto ret = _nud();
  if (!_is_type_resolved) { report_code_error(_token, "Must specify a type for the argument"); }
  return ret;
}

ASTArgDeclPtr ASTArgDecl::Create(const str &name, ASTTyPtr ty) {
  auto ret = std::make_shared<ASTArgDecl>(nullptr, 0);
  ret->_is_type_resolved = true;
  ret->_ty = ty;
  ret->_name = name;
  ret->_children.push_back(ASTStringLiteral::Create(name));
  ret->_children.push_back(ty);
  return ret;
}
