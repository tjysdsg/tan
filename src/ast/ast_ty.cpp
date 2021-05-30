#include "src/ast/ast_ty.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

ASTTypePtr ASTType::find_cache(Ty t, const vector<ASTTypePtr> &sub_tys, bool is_lvalue) {
  auto find = ASTType::_cache.find(t);
  if (find == ASTType::_cache.end()) { return nullptr; }
  if (find->second->_is_lvalue != is_lvalue) { return nullptr; }
  auto ret = find->second;

  if (sub_tys.size() != ret->get_children_size()) { return nullptr; }
  size_t idx = 0;
  for (const auto &sub : sub_tys) {
    auto t1 = sub;
    auto t2 = ret->get_child_at<ASTType>(idx);
    if (t1->_tyty != t2->_tyty) { return nullptr; }
    if (t1->_is_lvalue != t2->_is_lvalue) { return nullptr; }
    ++idx;
  }
  return ret;
}

bool ASTType::operator==(const ASTType &other) {
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

  if (get_children_size() > 0) {
    size_t n = get_children_size();
    if (n != other.get_children_size()) { return false; }
    for (size_t i = 0; i < n; ++i) {
      auto lhs = get_child_at<ASTType>(i);
      TAN_ASSERT(lhs);
      auto rhs = other.get_child_at<ASTType>(i);
      TAN_ASSERT(rhs);
      if (!lhs->operator==(*rhs)) { return false; }
    }
  }
  return true;
}

str ASTType::to_string(bool print_prefix) {
  str ret = "";
  if (print_prefix) {
    ret += "Type: ";
  }
  ret += _type_name;
  return ret;
}

bool ASTType::operator!=(const ASTType &other) { return !this->operator==(other); }

umap<str, Ty>ASTType::basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
        {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
        {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
        {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
        {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING}, {"char", Ty::CHAR},
        {"bool", Ty::BOOL},};

umap<str, Ty> ASTType::qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

ASTType::ASTType() {
  set_node_type(ASTNodeType::TY);
}
