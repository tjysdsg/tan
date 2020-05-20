#include "src/analysis/analysis.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_number_literal.h"
#include "compiler_session.h"
#include "compiler.h"

using namespace tanlang;

/// \section general

str Analyzer::get_name(ASTNodePtr p) { return p->_name; }

bool Analyzer::is_typed(ASTNodePtr p) { p->_is_typed; }

size_t Analyzer::get_n_children(ASTNodePtr p) { return p->_children.size(); }

/// \section types

str Analyzer::get_type_name(ASTNodePtr p) { return get_ty(p)->_type_name; }

ASTTyPtr Analyzer::get_ty(ASTNodePtr p) { return p->_ty; }

ASTTyPtr Analyzer::create_ty(Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue) {
  // TODO: find_cache
  // auto ret = find_cache(t, sub_tys, is_lvalue);
  // if (ret) { return ret; }
  auto ret = make_ptr<ASTTy>(nullptr, 0);
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  resolve(ret);
  return ret;
}

void Analyzer::resolve(ASTTyPtr p) {
  Ty base = TY_GET_BASE(p->_tyty);
  Ty qual = TY_GET_QUALIFIER(p->_tyty);
  if (p->_resolved) {
    if (base == Ty::STRUCT) {
      if (!p->_is_forward_decl) { return; }
    } else { return; }
  }
  p->_ty = p;
  /// resolve children if they are ASTTy
  for (auto c: p->_children) {
    auto t = ast_cast<ASTTy>(c);
    if (t && t->_type == ASTType::TY && !t->_resolved) { resolve(t); }
  }
  // FIXME: can't use p->_cs here, cuz some ty are created by Analyzer::create_ty() without its _cs being set
  auto *tm = Compiler::GetDefaultTargetMachine();
  switch (base) {
    case Ty::INT: {
      p->_size_bits = 32;
      p->_type_name = "i32";
      p->_is_int = true;
      p->_default_value.emplace<uint64_t>(0);
      if (TY_IS(qual, Ty::BIT8)) {
        p->_size_bits = 8;
        p->_type_name = "i8";
      } else if (TY_IS(qual, Ty::BIT16)) {
        p->_size_bits = 16;
        p->_type_name = "i16";
      } else if (TY_IS(qual, Ty::BIT64)) {
        p->_size_bits = 64;
        p->_type_name = "i64";
      }
      if (TY_IS(qual, Ty::UNSIGNED)) {
        p->_is_unsigned = true;
        if (p->_size_bits == 8) {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
        } else {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned;
        }
      } else {
        if (p->_size_bits == 8) {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed_char;
        } else {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed;
        }
      }
      break;
    }
    case Ty::CHAR:
      p->_type_name = "char";
      p->_size_bits = 8;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
      p->_is_unsigned = true;
      p->_default_value.emplace<uint64_t>(0);
      p->_is_int = true;
      break;
    case Ty::BOOL:
      p->_type_name = "bool";
      p->_size_bits = 1;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_boolean;
      p->_default_value.emplace<uint64_t>(0);
      p->_is_bool = true;
      break;
    case Ty::FLOAT:
      p->_type_name = "float";
      p->_size_bits = 32;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_float;
      p->_default_value.emplace<float>(0);
      p->_is_float = true;
      break;
    case Ty::DOUBLE:
      p->_type_name = "double";
      p->_size_bits = 64;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_float;
      p->_default_value.emplace<double>(0);
      p->_is_double = true;
      break;
    case Ty::STRING:
      p->_type_name = "u8*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_default_value.emplace<str>("");
      p->_align_bits = 8;
      p->_is_ptr = true;
      break;
    case Ty::VOID:
      p->_type_name = "void";
      p->_size_bits = 0;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed;
      break;
    case Ty::ENUM: {
      auto sub = ast_cast<ASTTy>(p->_children[0]);
      TAN_ASSERT(sub);
      p->_size_bits = sub->_size_bits;
      p->_align_bits = sub->_align_bits;
      p->_dwarf_encoding = sub->_dwarf_encoding;
      p->_default_value = sub->_default_value;
      p->_is_unsigned = sub->_is_unsigned;
      p->_is_int = sub->_is_int;
      p->_is_enum = true;
      /// _type_name, however, is set by ASTEnum::nud
      break;
    }
    case Ty::STRUCT: {
      /// align size is the max element size, if no element, 8 bits
      /// size is the number of elements * align size
      if (p->_is_forward_decl) {
        auto real = ast_cast<ASTTy>(p->_cs->get(p->_type_name));
        if (!real) { p->error("Incomplete type"); }
        *p = *real;
        p->_is_forward_decl = false;
      } else {
        p->_align_bits = 8;
        size_t n = p->_children.size();
        for (size_t i = 0; i < n; ++i) {
          auto et = ast_cast<ASTTy>(p->_children[i]);
          auto s = get_size_bits(et);
          if (s > p->_align_bits) { p->_align_bits = s; }
        }
        p->_size_bits = n * p->_align_bits;
        p->_is_struct = true;
      }
      break;
    }
    case Ty::ARRAY: {
      if (p->_children.empty()) { p->error("Invalid type"); }
      auto et = ast_cast<ASTTy>(p->_children[0]);
      auto s = ast_cast<ASTNumberLiteral>(p->_children[1]);
      TAN_ASSERT(et);
      TAN_ASSERT(s);
      p->_n_elements = p->_children.size();
      p->_type_name = "[" + get_type_name(et) + ", " + std::to_string(p->_n_elements) + "]";
      p->_is_ptr = true;
      p->_is_array = true;
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = get_size_bits(et);
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    case Ty::POINTER: {
      if (p->_children.empty()) { p->error("Invalid type"); }
      auto e = ast_cast<ASTTy>(p->_children[0]);
      TAN_ASSERT(e);
      resolve(e);
      p->_type_name = get_type_name(e) + "*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = get_size_bits(e);
      p->_is_ptr = true;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    default:
      p->error("Invalid type");
  }
  p->_resolved = true;
}

ASTTyPtr Analyzer::get_ptr_to(ASTTyPtr p) { return Analyzer::create_ty(Ty::POINTER, {get_ty(p)}, false); }

bool Analyzer::is_array(ASTTyPtr p) {
  resolve(p);
  return p->_is_array;
}

bool Analyzer::is_ptr(ASTTyPtr p) {
  resolve(p);
  return p->_is_ptr;
}

bool Analyzer::is_float(ASTTyPtr p) {
  resolve(p);
  return p->_is_float;
}

bool Analyzer::is_double(ASTTyPtr p) {
  resolve(p);
  return p->_is_double;
}

bool Analyzer::is_int(ASTTyPtr p) {
  resolve(p);
  return p->_is_int;
}

bool Analyzer::is_bool(ASTTyPtr p) {
  resolve(p);
  return p->_is_bool;
}

bool Analyzer::is_enum(ASTTyPtr p) {
  resolve(p);
  return p->_is_enum;
}

bool Analyzer::is_unsigned(ASTTyPtr p) {
  resolve(p);
  return p->_is_unsigned;
}

bool Analyzer::is_struct(ASTTyPtr p) {
  resolve(p);
  return p->_is_struct;
}

bool Analyzer::is_floating(ASTTyPtr p) {
  resolve(p);
  return p->_is_float || p->_is_double;
}

bool Analyzer::is_lvalue(ASTTyPtr p) {
  resolve(p);
  return p->_is_lvalue;
}

bool Analyzer::is_lvalue(ASTNodePtr p) { return is_lvalue(get_ty(p)); }

ASTTyPtr Analyzer::get_contained_ty(ASTTyPtr p) {
  if (p->_tyty == Ty::STRING) { return create_ty(Ty::CHAR, vector<ASTNodePtr>(), false); }
  else if (p->_is_ptr) {
    TAN_ASSERT(p->_children.size());
    auto ret = ast_cast<ASTTy>(p->_children[0]);
    TAN_ASSERT(ret);
    resolve(ret);
    return ret;
  } else { return nullptr; }
}

size_t Analyzer::get_size_bits(ASTTyPtr p) {
  resolve(p);
  return p->_size_bits;
}

ASTTyPtr Analyzer::get_struct_member_ty(ASTTyPtr p, size_t i) {
  TAN_ASSERT(p->_tyty == Ty::STRUCT);
  return get_ty(p->_children[i]);
}

size_t Analyzer::get_struct_member_index(ASTTyPtr p, str name) {
  auto search = p->_member_indices.find(name);
  if (search == p->_member_indices.end()) {
    p->error("Unknown member of struct '" + get_type_name(p) + "'");
  }
  return search->second;
}

void Analyzer::set_is_lvalue(ASTTyPtr p, bool is_lvalue) { p->_is_lvalue = is_lvalue; }
