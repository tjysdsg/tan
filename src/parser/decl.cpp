#include "parser.h"
#include <src/ast/ast_struct.h>
#include "src/ast/ast_expr.h"

namespace tanlang {

// TODO: merge ASTArgDecl::nud with ASTVarDecl::nud

size_t ASTArgDecl::nud(Parser *parser) {
  _end_index = _start_index;
  _children.push_back(parser->parse<ASTType::ID>(_end_index, true)); /// name
  parser->peek(_end_index, TokenType::PUNCTUATION, ":"); // TODO: type inference
  ++_end_index;
  /// type
  _ty = ast_cast<ASTTy>(parser->parse<ASTType::TY>(_end_index, true));
  _ty->set_is_lvalue(true);
  _children.push_back(_ty);
  return _end_index;
}

size_t ASTVarDecl::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "var"
  _children.push_back(parser->parse<ASTType::ID>(_end_index, true)); /// name
  parser->peek(_end_index, TokenType::PUNCTUATION, ":"); // TODO: type inference
  ++_end_index;
  /// type
  _ty = ast_cast<ASTTy>(parser->parse<ASTType::TY>(_end_index, true));
  _ty->set_is_lvalue(true);
  _children.push_back(_ty);
  return _end_index;
}

size_t ASTStruct::nud(Parser *parser) {
  _end_index = _start_index + 1; /// skip "struct"
  _children.push_back(parser->next_expression(_end_index)); // name
  auto comp_statements = parser->peek(_end_index);
  _end_index = comp_statements->parse(parser); // FIXME: struct declaration with no definition
  _children.insert(_children.begin() + 1, comp_statements->_children.begin(), comp_statements->_children.end());
  return _end_index;
}

} // namespace tanlang
