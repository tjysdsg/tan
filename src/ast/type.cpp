#include "src/ast/type.h"

using namespace tanlang;

StringType *Type::STRING_TYPE = new StringType();

Type *Type::get_canonical() {
  TAN_ASSERT(_canonical_type != this);
  if (!_canonical_type) { return this; }
  return _canonical_type;
}

bool Type::is_canonical() { return !_canonical_type; }

void Type::set_canonical(Type *t) { _canonical_type = t; }

PrimitiveType *PrimitiveType::Create(PrimitiveType::Kind kind) {
  auto it = CACHE.find(kind);
  if (it != CACHE.end()) {
    return it->second;
  } else {
    auto *ret = new PrimitiveType();
    ret->_kind = kind;
    ret->_type_name = TYPE_NAMES[kind];
    CACHE[kind] = ret;
    return ret;
  }
}

PrimitiveType *Type::GetVoidType() {
  return PrimitiveType::Create(PrimitiveType::VOID);
}

PrimitiveType *Type::GetBoolType() {
  return PrimitiveType::Create(PrimitiveType::BOOL);
}

PrimitiveType *Type::GetCharType() {
  return PrimitiveType::Create(PrimitiveType::CHAR);
}

PrimitiveType *Type::GetIntegerType(size_t bit_size, bool is_unsigned) {
  switch (bit_size) {
    case 8:
      if (is_unsigned) {
        return PrimitiveType::Create(PrimitiveType::U8);
      } else {
        return PrimitiveType::Create(PrimitiveType::I8);
      }
    case 16:
      if (is_unsigned) {
        return PrimitiveType::Create(PrimitiveType::U16);
      } else {
        return PrimitiveType::Create(PrimitiveType::I16);
      }
    case 32:
      if (is_unsigned) {
        return PrimitiveType::Create(PrimitiveType::U32);
      } else {
        return PrimitiveType::Create(PrimitiveType::I32);
      }
    case 64:
      if (is_unsigned) {
        return PrimitiveType::Create(PrimitiveType::U64);
      } else {
        return PrimitiveType::Create(PrimitiveType::I64);
      }
    default:
      TAN_ASSERT(false);
  }
}

PrimitiveType *Type::GetFloatType(size_t bit_size) {
  switch (bit_size) {
    case 32:
      return PrimitiveType::Create(PrimitiveType::F32);
    case 64:
      return PrimitiveType::Create(PrimitiveType::F64);
    default:
      TAN_ASSERT(false);
  }
}

StringType *Type::GetStringType() { return STRING_TYPE; }

PointerType *Type::GetPointerType(Type *pointee) {
  auto it = POINTER_TYPE_CACHE.find(pointee);
  if (it != POINTER_TYPE_CACHE.end()) {
    return it->second;
  } else {
    auto *ret = new PointerType(pointee);
    POINTER_TYPE_CACHE[pointee] = ret;
    return ret;
  }
}

ArrayType *Type::GetArrayType(Type *element_type, int size) {
  auto it = ARRAY_TYPE_CACHE.find({element_type, size});
  if (it != ARRAY_TYPE_CACHE.end()) {
    return it->second;
  } else {
    auto *ret = new ArrayType(element_type, size);
    ARRAY_TYPE_CACHE[{element_type, size}] = ret;
    return ret;
  }
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

bool Type::is_enum() {
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

PointerType::PointerType(Type *pointee_type) : _pointee_type(pointee_type) {
  _type_name = pointee_type->get_typename() + "*";
}

ArrayType::ArrayType(Type *element_type, int size) : _element_type(element_type), _size(size) {
  _type_name = element_type->get_typename() + "[" + std::to_string(size) + "]";
}

TypeRef::TypeRef(str name) {
  _type_name = name;
}

StringType::StringType() {
  _type_name = "str";
}
