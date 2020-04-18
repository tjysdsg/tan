#include "src/ast/ast_array.h"
#include "src/ast/common.h"

namespace tanlang {

Value *ASTArrayLiteral::codegen(CompilerSession *compiler_session) {
  using llvm::Constant;
  auto sub = ast_cast<ASTLiteral>(_children[0]);
  sub->codegen(compiler_session);
  _e_llvm_type = sub->to_llvm_type(compiler_session);
  Type *int_t = compiler_session->get_builder()->getInt32Ty();
  size_t n = _children.size();
  auto *size = ConstantInt::get(int_t, n);
  _llvm_value = compiler_session->get_builder()->CreateAlloca(_e_llvm_type, 0, size);
  for (size_t i = 0; i < n; ++i) {
    auto *idx = ConstantInt::get(int_t, i);
    auto *e_val = _children[i]->codegen(compiler_session);
    auto *e_ptr = compiler_session->get_builder()->CreateGEP(_llvm_value, idx);
    compiler_session->get_builder()->CreateStore(e_val, e_ptr);
  }
  _llvm_type = _llvm_value->getType();
  return _llvm_value;
}

llvm::Value *ASTArrayLiteral::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

std::string ASTArrayLiteral::get_type_name() const {
  std::string ret = "[";
  size_t i = 0;
  size_t n = _children.size();
  for (auto c : _children) {
    ret += c->get_type_name();
    if (i < n - 1) { ret += ", "; }
    ++i;
  }
  return ret + "]";
}

llvm::Type *ASTArrayLiteral::to_llvm_type(CompilerSession *) const {
  return _llvm_type;
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

llvm::Type *ASTArrayLiteral::get_element_llvm_type(CompilerSession *) const {
  return _e_llvm_type;
}

size_t ASTArrayLiteral::get_n_elements() const {
  return _children.size();
}

llvm::Type *ASTArrayLiteral::get_gep_base_type(CompilerSession *compiler_session) const {
  if (_children[0]->_type == ASTType::ARRAY_LITERAL) {
    auto sub = ast_cast<ASTArrayLiteral>(_children[0]);
    return llvm::ArrayType::get(sub->get_element_llvm_type(compiler_session), sub->get_n_elements());
  } else {
    assert(_children[0]->is_typed());
    return _children[0]->to_llvm_type(compiler_session);
  }
}

} // namespace tanlang
