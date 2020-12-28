#include "src/ast/ast_ty.h"
#include "src/analysis/analysis.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/common.h"

using namespace tanlang;

ASTTyPtr ASTTy::find_cache(Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue) {
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

Metadata *ASTTy::to_llvm_meta(CompilerSession *cs) {
  resolve(this->ptr_from_this());
  Ty base = TY_GET_BASE(_tyty);
  // TODO: Ty qual = TY_GET_QUALIFIER(_tyty);
  DIType *ret = nullptr;
  switch (base) {
    case Ty::CHAR:
    case Ty::INT:
    case Ty::BOOL:
    case Ty::FLOAT:
    case Ty::VOID:
    case Ty::DOUBLE:
    case Ty::ENUM:
      ret = cs->_di_builder->createBasicType(_type_name, _size_bits, _dwarf_encoding);
      break;
    case Ty::STRING: {
      auto *e_di_type = cs->_di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = cs->_di_builder->createPointerType(e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    case Ty::STRUCT: {
      DIFile *di_file = cs->get_di_file();
      size_t n = _children.size();
      vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        auto e = _children[i]; // ASTVarDecl
        elements.push_back(get_ty(e)->to_llvm_meta(cs));
      }
      ret = cs->_di_builder
          ->createStructType(cs->get_current_di_scope(),
              _type_name,
              di_file,
              (unsigned) _token->l,
              _size_bits,
              (unsigned) _align_bits,
              DINode::DIFlags::FlagZero,
              nullptr,
              cs->_di_builder->getOrCreateArray(elements),
              0,
              nullptr,
              _type_name);
      break;
    }
    case Ty::ARRAY:
    case Ty::POINTER: {
      auto e = ast_cast<ASTTy>(_children[0]);
      auto *e_di_type = e->to_llvm_meta(cs);
      ret = cs->_di_builder
          ->createPointerType((DIType *) e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

bool ASTTy::operator==(const ASTTy &other) {
  #define CHECK(val) if (this->val != other.val) { return false; }
  CHECK(_size_bits)
  CHECK(_align_bits)
  CHECK(_is_ptr)
  CHECK(_is_float)
  CHECK(_is_double)
  CHECK(_is_int)
  CHECK(_is_unsigned)
  CHECK(_is_struct)
  CHECK(_is_bool)
  #undef CHECK

  if (!_children.empty()) {
    size_t n = _children.size();
    if (n != other._children.size()) { return false; }
    for (size_t i = 0; i < n; ++i) {
      return ast_cast<ASTTy>(_children[i])->operator==(*ast_cast<ASTTy>(other._children[i]));
    }
  }
  return true;
}

str ASTTy::to_string(bool print_prefix) { return ASTNode::to_string(print_prefix) + " " + _type_name; }

bool ASTTy::operator!=(const ASTTy &other) { return !this->operator==(other); }

ASTTy &ASTTy::operator=(const ASTTy &other) {
  _tyty = other._tyty;
  _default_value = other._default_value;
  _type_name = other._type_name;
  _children = other._children;
  _size_bits = other._size_bits;
  _align_bits = other._align_bits;
  _dwarf_encoding = other._dwarf_encoding;
  _is_ptr = other._is_ptr;
  _is_float = other._is_float;
  _is_array = other._is_array;
  _is_int = other._is_int;
  _is_unsigned = other._is_unsigned;
  _is_struct = other._is_struct;
  _is_bool = other._is_bool;
  _is_enum = other._is_enum;
  _is_lvalue = other._is_lvalue;
  _is_forward_decl = other._is_forward_decl;
  return const_cast<ASTTy &>(*this);
}

ASTTy &ASTTy::operator=(ASTTy &&other) {
  _tyty = other._tyty;
  _default_value = other._default_value;
  _type_name = other._type_name;
  _children = other._children;
  _size_bits = other._size_bits;
  _align_bits = other._align_bits;
  _dwarf_encoding = other._dwarf_encoding;
  _is_ptr = other._is_ptr;
  _is_float = other._is_float;
  _is_array = other._is_array;
  _is_int = other._is_int;
  _is_unsigned = other._is_unsigned;
  _is_struct = other._is_struct;
  _is_bool = other._is_bool;
  _is_enum = other._is_enum;
  _is_lvalue = other._is_lvalue;
  _is_forward_decl = other._is_forward_decl;
  return const_cast<ASTTy &>(*this);
}

ASTTy::ASTTy() : ASTNode(ASTType::TY, 0) {}
