#include "src/analysis/type_system.h"
#include "src/ast/ast_type.h"
#include "src/ast/decl.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

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

  if (_sub_types.size() > 0) {
    size_t n = _sub_types.size();
    if (n != other._sub_types.size()) { return false; }
    for (size_t i = 0; i < n; ++i) {
      ASTType *lhs = _sub_types[i];
      ASTType *rhs = other._sub_types[i];
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

ASTType::ASTType(SourceIndex loc) : ASTBase(ASTNodeType::TY, loc, 0) {}

ASTType *ASTType::Create(CompilerSession *cs, SourceIndex loc) {
  auto *ret = new ASTType(loc);
  ret->_cs = cs;
  return ret;
}

ASTType *ASTType::CreateAndResolve(CompilerSession *cs,
    SourceIndex loc,
    Ty t,
    vector<ASTType *> sub_tys,
    bool is_lvalue,
    std::function<void(ASTType *)> attribute_setter) {
  // TODO: cache
  auto ret = new ASTType(loc);
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_sub_types.insert(ret->_sub_types.begin(), sub_tys.begin(), sub_tys.end());
  ret->_cs = cs;
  if (attribute_setter) {
    attribute_setter(ret);
  }
  TypeSystem::ResolveTy(cs, ret);
  return ret;
}

Ty ASTType::get_ty() const { return _tyty; }

void ASTType::set_ty(Ty ty) { _tyty = ty; }

const str &ASTType::get_type_name() const { return must_get_canonical_type()->_type_name; }

void ASTType::set_type_name(const str &type_name) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_type_name = type_name;
}

Type *ASTType::get_llvm_type() const { return must_get_canonical_type()->_llvm_type; }

void ASTType::set_llvm_type(Type *llvm_type) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_llvm_type = llvm_type;
}

size_t ASTType::get_size_bits() const { return must_get_canonical_type()->_size_bits; }

void ASTType::set_size_bits(size_t size_bits) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_size_bits = size_bits;
}

size_t ASTType::get_align_bits() const { return must_get_canonical_type()->_align_bits; }

void ASTType::set_align_bits(size_t align_bits) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_align_bits = align_bits;
}

unsigned int ASTType::get_dwarf_encoding() const { return must_get_canonical_type()->_dwarf_encoding; }

void ASTType::set_dwarf_encoding(unsigned int dwarf_encoding) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_dwarf_encoding = dwarf_encoding;
}

bool ASTType::is_ptr() const { return must_get_canonical_type()->_is_ptr; }

void ASTType::set_is_ptr(bool is_ptr) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_ptr = is_ptr;
}

bool ASTType::is_float() const { return must_get_canonical_type()->_is_float; }

void ASTType::set_is_float(bool is_float) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_float = is_float;
}

bool ASTType::is_array() const { return must_get_canonical_type()->_is_array; }

void ASTType::set_is_array(bool is_array) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_array = is_array;
}

size_t ASTType::get_array_size() const { return must_get_canonical_type()->_array_size; }

void ASTType::set_array_size(size_t array_size) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_array_size = array_size;
}

bool ASTType::is_int() const { return must_get_canonical_type()->_is_int; }

void ASTType::set_is_int(bool is_int) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_int = is_int;
}

bool ASTType::is_unsigned() const { return must_get_canonical_type()->_is_unsigned; }

void ASTType::set_is_unsigned(bool is_unsigned) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_unsigned = is_unsigned;
}

bool ASTType::is_struct() const { return must_get_canonical_type()->_is_struct; }

void ASTType::set_is_struct(bool is_struct) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_struct = is_struct;
}

bool ASTType::is_bool() const { return must_get_canonical_type()->_is_bool; }

void ASTType::set_is_bool(bool is_bool) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_bool = is_bool;
}

bool ASTType::is_enum() const { return must_get_canonical_type()->_is_enum; }

void ASTType::set_is_enum(bool is_enum) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_enum = is_enum;
}

bool ASTType::is_resolved() const { return must_get_canonical_type()->_resolved; }

void ASTType::set_resolved(bool resolved) { must_get_canonical_type()->_resolved = resolved; }

bool ASTType::is_forward_decl() const { return must_get_canonical_type()->_is_forward_decl; }

void ASTType::set_is_forward_decl(bool is_forward_decl) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_is_forward_decl = is_forward_decl;
}

vector<ASTType *> &ASTType::get_sub_types() { return must_get_canonical_type()->_sub_types; }

void ASTType::set_sub_types(const vector<ASTType *> &sub_types) {
  no_modifications_on_type_reference();
  must_get_canonical_type()->_sub_types = sub_types;
}

// FIXME: let the canonical type be observable, instead of looking up a table every time
ASTType *ASTType::get_canonical_type() const {
  TAN_ASSERT(_cs);
  if (_tyty != Ty::TYPE_REF) {
    return (ASTType *) this;
  }

  Decl *decl = _cs->get_type_decl(_cs->get_source_manager()->get_token_str(_loc));
  if (decl) {
    return decl->get_type();
  }
  return nullptr;
}

ASTType *ASTType::must_get_canonical_type() const {
  ASTType *type = get_canonical_type();
  if (!type) {
    TAN_ASSERT(_type_name != "");
    report_error(_cs->_filename, _cs->get_source_manager()->get_token(_loc), "Invalid type name");
  }
  return type;
}

void ASTType::no_modifications_on_type_reference() const { TAN_ASSERT(_tyty != Ty::TYPE_REF); }

bool ASTType::is_lvalue() const { return _is_lvalue; }

void ASTType::set_is_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }

Constructor *ASTType::get_constructor() const { return must_get_canonical_type()->_constructor; }

void ASTType::set_constructor(Constructor *constructor) {
  no_modifications_on_type_reference();
  _constructor = constructor;
}
