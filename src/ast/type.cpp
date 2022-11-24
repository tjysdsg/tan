#include "src/ast/type.h"

using namespace tanlang;

Type *Type::get_canonical() {
  TAN_ASSERT(_canonical_type != this);
  if (!_canonical_type) { return this; }
  return _canonical_type;
}

bool Type::is_canonical() { return !_canonical_type; }

void Type::set_canonical(Type *t) { _canonical_type = t; }

StringType *Type::GetStringType(SrcLoc loc) { return new StringType(loc); }

PrimitiveType *PrimitiveType::Create(SrcLoc loc, PrimitiveType::Kind kind) {
  auto *ret = new PrimitiveType(loc);
  ret->_kind = kind;
  ret->_type_name = TYPE_NAMES[kind];
  return ret;
}

PrimitiveType *Type::GetVoidType(SrcLoc loc) {
  return PrimitiveType::Create(loc, PrimitiveType::VOID);
}

PrimitiveType *Type::GetBoolType(SrcLoc loc) {
  return PrimitiveType::Create(loc, PrimitiveType::BOOL);
}

PrimitiveType *Type::GetCharType(SrcLoc loc) {
  return PrimitiveType::Create(loc, PrimitiveType::CHAR);
}

PrimitiveType *Type::GetIntegerType(SrcLoc loc, size_t bit_size, bool is_unsigned) {
  switch (bit_size) {
    case 8:
      if (is_unsigned) {
        return PrimitiveType::Create(loc, PrimitiveType::U8);
      } else {
        return PrimitiveType::Create(loc, PrimitiveType::I8);
      }
    case 16:
      if (is_unsigned) {
        return PrimitiveType::Create(loc, PrimitiveType::U16);
      } else {
        return PrimitiveType::Create(loc, PrimitiveType::I16);
      }
    case 32:
      if (is_unsigned) {
        return PrimitiveType::Create(loc, PrimitiveType::U32);
      } else {
        return PrimitiveType::Create(loc, PrimitiveType::I32);
      }
    case 64:
      if (is_unsigned) {
        return PrimitiveType::Create(loc, PrimitiveType::U64);
      } else {
        return PrimitiveType::Create(loc, PrimitiveType::I64);
      }
    default:
      TAN_ASSERT(false);
  }
}

PrimitiveType *Type::GetFloatType(SrcLoc loc, size_t bit_size) {
  switch (bit_size) {
    case 32:
      return PrimitiveType::Create(loc, PrimitiveType::F32);
    case 64:
      return PrimitiveType::Create(loc, PrimitiveType::F64);
    default:
      TAN_ASSERT(false);
  }
}

PointerType *Type::GetPointerType(SrcLoc loc, Type *pointee) {
  return new PointerType(loc, pointee);
}

ArrayType *Type::GetArrayType(SrcLoc loc, Type *element_type, int size) {
  return new ArrayType(loc, element_type, size);
}

bool Type::is_primitive() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_pointer() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_array() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_string() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_struct() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_ref() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_float() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_int() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_unsigned() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_bool() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_void() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

bool Type::is_char() {
  TAN_ASSERT(!_canonical_type);
  return false;
}

int PrimitiveType::get_size_bits() {
  TAN_ASSERT(_kind != VOID);
  return SIZE_BITS[_kind];
}

int PrimitiveType::get_align_bits() {
  TAN_ASSERT(_kind != VOID);
  return SIZE_BITS[_kind]; // the same as their sizes
}

PointerType::PointerType(SrcLoc loc, Type *pointee_type) : Type(loc), _pointee_type(pointee_type) {
  _type_name = pointee_type->get_typename() + "*";
}

ArrayType::ArrayType(SrcLoc loc, Type *element_type, int size) : Type(loc), _element_type(element_type), _size(size) {
  _type_name = element_type->get_typename() + "[" + std::to_string(size) + "]";
}

TypeRef::TypeRef(SrcLoc loc, str name) : Type(loc) {
  _type_name = name;
}

StringType::StringType(SrcLoc loc) : Type(loc) {
  _type_name = "str";
}
