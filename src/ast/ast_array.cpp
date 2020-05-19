#include "src/ast/ast_array.h"
#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_ty.h"
#include "src/common.h"
#include "src/type_system.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

size_t ASTArrayLiteral::nud() {
  _end_index = _start_index + 1; /// skip '['
  if (_parser->at(_end_index)->value == "]") { error("Empty array"); }
  ASTType element_type = ASTType::INVALID;
  while (!_parser->eof(_end_index)) {
    if (_parser->at(_end_index)->value == ",") {
      ++_end_index;
      continue;
    } else if (_parser->at(_end_index)->value == "]") {
      ++_end_index;
      break;
    }
    auto node = _parser->peek(_end_index);
    if (!node) { error(_end_index, "Unexpected token"); }
    /// check whether element types are the same
    if (element_type == ASTType::INVALID) { element_type = node->_type; }
    else {
      if (element_type != node->_type) {
        error(_end_index, "All elements in an array must have the same type");
      }
    }
    if (is_ast_type_in(node->_type, TypeSystem::LiteralTypes)) {
      if (node->_type == ASTType::ARRAY_LITERAL) { ++_end_index; }
      _end_index = node->parse(_parser, _cs);
      _children.push_back(node);
    } else { error(_end_index, "Expect literals"); }
  }

  auto size = std::make_shared<ASTNumberLiteral>(get_n_elements(), 0);
  vector<ASTNodePtr> sub_tys{};
  sub_tys.reserve(get_n_elements());
  std::for_each(_children.begin(),
      _children.end(),
      [&sub_tys](const ASTNodePtr &e) { sub_tys.push_back(e->get_ty()); });
  _ty = ASTTy::Create(Ty::ARRAY, sub_tys);
  return _end_index;
}

Value *ASTArrayLiteral::_codegen(CompilerSession *cs) {
  _llvm_value = _ty->get_llvm_value(cs);
  return _llvm_value;
}

str ASTArrayLiteral::to_string(bool print_prefix) {
  str ret;
  if (print_prefix) { ret = ASTLiteral::to_string(true) + " "; }
  ret += "[";
  size_t i = 0;
  size_t n = _children.size();
  for (auto c : _children) {
    ret += c->to_string(false);
    if (i < n - 1) { ret += ", "; }
    ++i;
  }
  return ret + "]";
}

Type *ASTArrayLiteral::get_element_llvm_type(CompilerSession *cs) {
  return get_ty()->get_contained_ty()->to_llvm_type(cs);
}

size_t ASTArrayLiteral::get_n_elements() { return _children.size(); }

ASTArrayLiteral::ASTArrayLiteral(Token *t, size_t ti) : ASTLiteral(ASTType::ARRAY_LITERAL, 0, 0, t, ti) {}

} // namespace tanlang
