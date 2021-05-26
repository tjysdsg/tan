#include "src/ast/ast_ty.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

ASTTyPtr ASTTy::find_cache(Ty t, const vector<ASTTyPtr> &sub_tys, bool is_lvalue) {
  auto find = ASTTy::_cache.find(t);
  if (find == ASTTy::_cache.end()) { return nullptr; }
  if (find->second->_is_lvalue != is_lvalue) { return nullptr; }
  auto ret = find->second;

  if (sub_tys.size() != ret->_children.size()) { return nullptr; }
  size_t idx = 0;
  for (const auto &sub : sub_tys) {
    auto t1 = sub;
    auto t2 = ret->_children[idx];
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
      auto lhs = _children.at(i);
      TAN_ASSERT(lhs);
      auto rhs = other._children.at(i);
      TAN_ASSERT(rhs);
      if (!lhs->operator==(*rhs)) { return false; }
    }
  }
  return true;
}

str ASTTy::to_string(bool print_prefix) {
  str ret = "";
  if (print_prefix) {
    ret += "Type: ";
  }
  ret += _type_name;
  return ret;
}

bool ASTTy::operator!=(const ASTTy &other) { return !this->operator==(other); }

umap<str, Ty>ASTTy::basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
        {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
        {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
        {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
        {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING}, {"char", Ty::CHAR},
        {"bool", Ty::BOOL},};

umap<str, Ty> ASTTy::qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

ASTTy::ASTTy() {
  set_node_type(ASTType::TY);
}
