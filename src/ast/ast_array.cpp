#include "src/ast/ast_array.h"
#include "src/common.h"
#include "src/type_system.h"
#include "compiler_session.h"

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

ASTArrayLiteral::ASTArrayLiteral(Token *token, size_t token_index) : ASTLiteral(ASTType::ARRAY_LITERAL,
    0,
    0,
    token,
    token_index) {}

std::shared_ptr<ASTTy> ASTArrayLiteral::get_ty() const {
  // TODO: optimize this
  std::vector<std::shared_ptr<ASTTy>> sub_tys{};
  for (const auto &c : _children) { sub_tys.push_back(c->get_ty()); }
  auto ret = ASTTy::Create(Ty::ARRAY, false, sub_tys);
  return ret;
}

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
  return _end_index;
}

} // namespace tanlang
