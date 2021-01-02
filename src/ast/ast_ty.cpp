#include "src/ast/ast_ty.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

ASTTyPtr ASTTy::find_cache(Ty t, const vector<ASTNodePtr> &sub_tys, bool is_lvalue) {
  auto find = ASTTy::_cache.find(t);
  if (find == ASTTy::_cache.end()) { return nullptr; }
  if (find->second->_is_lvalue != is_lvalue) { return nullptr; }
  auto ret = find->second;

  if (sub_tys.size() != ret->_children.size()) { return nullptr; }
  size_t idx = 0;
  for (const auto &sub : sub_tys) {
    auto t1 = ast_cast<ASTTy>(sub);
    auto t2 = ast_cast<ASTTy>(ret->_children[idx]);
    if (t1->_tyty != t2->_tyty) { return nullptr; }
    if (t1->_is_lvalue != t2->_is_lvalue) { return nullptr; }
    ++idx;
  }
  return ret;
}

bool ASTTy::operator==(const ASTTy &other) {
  #define CHECK(val) if (this->val != other.val) { return false; }
  CHECK(_size_bits)
  CHECK(_align_bits)
  CHECK(_is_ptr)
  CHECK(_is_float)
  CHECK(_is_array);
  CHECK(_array_size);
  CHECK(_is_int)
  CHECK(_is_unsigned)
  CHECK(_is_struct)
  CHECK(_is_bool)
  CHECK(_is_enum);
  #undef CHECK

  if (!_children.empty()) {
    size_t n = _children.size();
    if (n != other._children.size()) { return false; }
    for (size_t i = 0; i < n; ++i) {
      auto lhs = ast_cast<ASTTy>(_children[i]);
      TAN_ASSERT(lhs);
      auto rhs = ast_cast<ASTTy>(other._children[i]);
      TAN_ASSERT(rhs);
      if (!lhs->operator==(*rhs)) { return false; }
    }
  }
  return true;
}

str ASTTy::to_string(bool print_prefix) { return ASTNode::to_string(print_prefix) + " " + _type_name; }

bool ASTTy::operator!=(const ASTTy &other) { return !this->operator==(other); }
