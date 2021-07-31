#include "src/analysis/type_system.h"
#include "src/ast/ast_type.h"
#include "src/ast/decl.h"
#include "src/ast/ast_context.h"
#include "src/llvm_include.h"
#include "token.h"

using namespace tanlang;

bool ASTType::operator==(const ASTType &other) {
  #define CHECK(val) if (this->val != other.val) return false
  CHECK(_size_bits);
  // FIXME: align_size of the pointer to a struct is broken
  //   CHECK(_align_bits);
  CHECK(is_ptr());
  CHECK(_is_float);
  CHECK(_is_array);
  CHECK(_array_size);
  CHECK(_is_int);
  CHECK(_is_unsigned);
  CHECK(_is_struct);
  CHECK(_is_bool);
  CHECK(_is_enum);
  #undef CHECK

  size_t n = _sub_types.size();
  if (n != other._sub_types.size()) { return false; }
  if (!_sub_types.empty()) {
    for (size_t i = 0; i < n; ++i) {
      ASTType *lhs = _sub_types[i];
      ASTType *rhs = other._sub_types[i];
      if (!lhs->operator==(*rhs)) { return false; }
    }
  }
  return true;
}

str ASTType::to_string(bool print_prefix) const {
  str ret;
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

ASTType *ASTType::Create(ASTContext *ctx, SourceIndex loc) {
  auto *ret = new ASTType(loc);
  ret->_ctx = ctx;
  return ret;
}

ASTType *ASTType::CreateAndResolve(ASTContext *ctx,
    SourceIndex loc,
    Ty t,
    vector<ASTType *> sub_tys,
    bool is_lvalue,
    const std::function<void(ASTType *)> &attribute_setter) {
  // TODO: cache
  auto ret = new ASTType(loc);
  ret->_ty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_sub_types.insert(ret->_sub_types.begin(), sub_tys.begin(), sub_tys.end());
  ret->_ctx = ctx;
  if (attribute_setter) {
    attribute_setter(ret);
  }
  TypeSystem::ResolveTy(ctx, ret);
  return ret;
}

ASTType *ASTType::GetVoidType(ASTContext *ctx, SourceIndex loc) {
  return ASTType::CreateAndResolve(ctx, loc, Ty::VOID);
}

ASTType *ASTType::GetI32Type(ASTContext *ctx, SourceIndex loc, bool lvalue) {
  return ASTType::CreateAndResolve(ctx, loc, TY_OR(Ty::INT, Ty::BIT32), {}, lvalue);
}

ASTType *ASTType::GetU32Type(ASTContext *ctx, SourceIndex loc, bool lvalue) {
  return ASTType::CreateAndResolve(ctx, loc, TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED), {}, lvalue);
}

ASTType *ASTType::GetI8Type(ASTContext *ctx, SourceIndex loc, bool lvalue) {
  return ASTType::CreateAndResolve(ctx, loc, TY_OR(Ty::INT, Ty::BIT8), {}, lvalue);
}

ASTType *ASTType::GetBoolType(ASTContext *ctx, SourceIndex loc, bool lvalue) {
  return ASTType::CreateAndResolve(ctx, loc, Ty::BOOL, {}, lvalue);
}

Ty ASTType::get_ty() const { return _ty; }

void ASTType::set_ty(Ty ty) { _ty = ty; }

const str &ASTType::get_type_name() const { return _type_name; }

void ASTType::set_type_name(const str &type_name) { _type_name = type_name; }

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

// TODO: array/string is not exactly a pointer
bool ASTType::is_ptr() const {
  return must_get_canonical_type()->_ty == Ty::POINTER || must_get_canonical_type()->_ty == Ty::ARRAY
      || must_get_canonical_type()->_ty == Ty::STRING;
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

bool ASTType::is_numeric() const { return is_float() || is_int(); }

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
  TAN_ASSERT(_ctx);
  if (_ty != Ty::TYPE_REF) {
    return (ASTType *) this;
  }

  Decl *decl = _ctx->get_type_decl(_ctx->get_source_manager()->get_token_str(_loc));
  if (decl) {
    return decl->get_type();
  }
  return nullptr;
}

ASTType *ASTType::must_get_canonical_type() const {
  ASTType *type = get_canonical_type();
  if (!type) {
    TAN_ASSERT(!_type_name.empty());
    Error err(_ctx->_filename, _ctx->get_source_manager()->get_token(_loc), "Invalid type name");
    err.raise();
  }
  return type;
}

void ASTType::no_modifications_on_type_reference() const { TAN_ASSERT(_ty != Ty::TYPE_REF); }

bool ASTType::is_lvalue() const { return _is_lvalue; }

void ASTType::set_is_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }

Constructor *ASTType::get_constructor() const { return must_get_canonical_type()->_constructor; }

void ASTType::set_constructor(Constructor *constructor) {
  no_modifications_on_type_reference();
  _constructor = constructor;
}

ASTType *ASTType::get_contained_ty() const {
  if (get_ty() == Ty::STRING) {
    return ASTType::CreateAndResolve(_ctx, get_loc(), Ty::CHAR, {}, false);
  } else if (is_ptr()) {
    TAN_ASSERT(!get_canonical_type()->_sub_types.empty());
    auto ret = get_canonical_type()->_sub_types[0];
    TAN_ASSERT(ret);
    return ret;
  } else {
    return nullptr;
  }
}

ASTType *ASTType::get_ptr_to() const {
  return ASTType::CreateAndResolve(_ctx, get_loc(), Ty::POINTER, {(ASTType *) this}, false);
}

vector<ASTBase *> ASTType::get_children() const {
  vector<ASTBase *> ret = {};
  ret.reserve(_sub_types.size());
  std::for_each(_sub_types.begin(), _sub_types.end(), [&](ASTType *t) { ret.push_back(t); });
  return ret;
}
