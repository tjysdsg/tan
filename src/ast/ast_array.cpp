#include "src/ast/ast_array.h"
#include "src/ast/common.h"

namespace tanlang {

Value *ASTArrayLiteral::codegen(CompilerSession *compiler_session) {
  using llvm::Constant;
  auto sub = ast_cast<ASTLiteral>(_children[0]);
  sub->codegen(compiler_session);
  _e_llvm_type = sub->to_llvm_type(compiler_session);
  std::vector<Constant *> constants;
  size_t n = _children.size();
  constants.reserve(n);
  for (size_t i = 0; i < n; ++i) {
    constants.push_back((Constant *) _children[i]->codegen(compiler_session));
  }
  Value *val = llvm::ConstantArray::get((llvm::ArrayType *) _e_llvm_type, constants);
  Value *ret = compiler_session->get_builder()
                               ->CreateAlloca(_e_llvm_type,
                                              ConstantInt::get(compiler_session->get_builder()->getInt32Ty(),
                                                               _children.size()));
  compiler_session->get_builder()->CreateStore(val, ret);
  _llvm_value = ret;
  _llvm_type = ret->getType();
  return ret;
}

llvm::Value *ASTArrayLiteral::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

std::string ASTArrayLiteral::get_type_name() const {
  std::string ret = "[";
  size_t i = 0;
  size_t n = _children.size();
  for (auto c : _children) {
    ret += ast_cast<ASTLiteral>(c)->get_type_name();
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
    auto sub = ast_cast<Typed>(_children[0]);
    return sub->to_llvm_type(compiler_session);
  }
}

} // namespace tanlang
