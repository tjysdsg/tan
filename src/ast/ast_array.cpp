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
  if (_parser->at(_end_index)->value == "]") { /// empty array
    ++_end_index;
    return _end_index;
  }
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
    if (!node) { report_code_error(_token, "Unexpected token"); }
    /// check whether element types are the same
    if (element_type == ASTType::INVALID) {
      element_type = node->_type;
    } else {
      if (element_type != node->_type) {
        report_code_error(_token, "All elements in an array must have the same type");
      }
    }
    if (is_ast_type_in(node->_type, TypeSystem::LiteralTypes)) {
      if (node->_type == ASTType::ARRAY_LITERAL) { ++_end_index; }
      _end_index = node->parse(_parser, _cs);
      _children.push_back(node);
    } else {
      report_code_error(_token, "Expect literals");
    }
  }

  // TODO: set default value of ASTTy
  auto size = std::make_shared<ASTNumberLiteral>(get_n_elements(), 0);
  std::vector<ASTNodePtr> sub_tys{_children[0]->get_ty(), size};
  _ty = ASTTy::Create(Ty::ARRAY, false, sub_tys);
  return _end_index;
}

Value *ASTArrayLiteral::codegen(CompilerSession *cs) {
  using llvm::Constant;
  auto sub = ast_cast<ASTLiteral>(_children[0]);
  sub->codegen(cs);
  Type *int_t = cs->get_builder()->getInt32Ty();
  size_t n = _children.size();
  auto *size = ConstantInt::get(int_t, n);
  _llvm_value = cs->get_builder()->CreateAlloca(get_element_llvm_type(cs), 0, size);
  for (size_t i = 0; i < n; ++i) {
    auto *idx = ConstantInt::get(int_t, i);
    auto *e_val = _children[i]->codegen(cs);
    auto *e_ptr = cs->get_builder()->CreateGEP(_llvm_value, idx);
    cs->get_builder()->CreateStore(e_val, e_ptr);
  }
  return _llvm_value;
}

std::string ASTArrayLiteral::to_string(bool print_prefix) const {
  std::string ret;
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

Type *ASTArrayLiteral::get_element_llvm_type(CompilerSession *cs) const {
  return get_ty()->get_contained_ty()->to_llvm_type(cs);
}

size_t ASTArrayLiteral::get_n_elements() const {
  return _children.size();
}

ASTArrayLiteral::ASTArrayLiteral(Token *t, size_t ti) : ASTLiteral(ASTType::ARRAY_LITERAL, 0, 0, t, ti) {}

} // namespace tanlang
