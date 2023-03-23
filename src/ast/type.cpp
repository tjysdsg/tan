#include "ast/type.h"
#include <bit>

using namespace tanlang;

StringType *Type::STRING_TYPE = new StringType();

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

PrimitiveType *Type::GetVoidType() { return PrimitiveType::Create(PrimitiveType::VOID); }

PrimitiveType *Type::GetBoolType() { return PrimitiveType::Create(PrimitiveType::BOOL); }

PrimitiveType *Type::GetCharType() { return PrimitiveType::Create(PrimitiveType::CHAR); }

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
    TAN_ASSERT(it->second->is_pointer() && it->second->get_pointee() == pointee);
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

// TODO IMPORTANT: cache function types
FunctionType *Type::GetFunctionType(Type *ret_type, const vector<Type *> &arg_types) {
  return new FunctionType(ret_type, arg_types);
}

StructType *Type::GetStructType(const str &name, const vector<Type *> &member_types) {
  auto it = NAMED_TYPE_CACHE.find(name);
  if (it != NAMED_TYPE_CACHE.end()) {
    auto *t = (StructType *)it->second;
    TAN_ASSERT(t->is_struct());
    t->_member_types = member_types; /// update forward declaration
    return t;
  } else {
    auto *ret = new StructType(name, member_types);
    NAMED_TYPE_CACHE[name] = ret;
    return ret;
  }
}

TypeRef *Type::GetTypeRef(const str &name) { return new TypeRef(name); }

IncompleteType *Type::GetIncompleteType() { return new IncompleteType(); }

bool Type::is_primitive() { return false; }

bool Type::is_pointer() { return false; }

bool Type::is_array() { return false; }

bool Type::is_string() { return false; }

bool Type::is_struct() { return false; }

bool Type::is_function() { return false; }

bool Type::is_ref() { return false; }

bool Type::is_float() { return false; }

bool Type::is_int() { return false; }

bool Type::is_num() { return false; }

bool Type::is_unsigned() { return false; }

bool Type::is_bool() { return false; }

bool Type::is_void() { return false; }

bool Type::is_char() { return false; }

int Type::get_align_bits() {
  TAN_ASSERT(false);
  return 0;
}

int Type::get_size_bits() {
  TAN_ASSERT(false);
  return 0;
}

vector<Type *> Type::children() const { TAN_ASSERT(false); }

bool Type::is_resolved() const {
  bool resolved = true;
  auto children = this->children();
  for (auto *c : children) {
    if (!c->is_resolved()) {
      resolved = false;
    }
  }
  return resolved;
}

int PrimitiveType::get_size_bits() { return SIZE_BITS[_kind]; }

int PrimitiveType::get_align_bits() {
  TAN_ASSERT(_kind != VOID);
  return SIZE_BITS[_kind]; // the same as their sizes
}

PointerType::PointerType(Type *pointee_type) : _pointee_type(pointee_type) {
  _type_name = pointee_type->get_typename() + "*";
}

vector<Type *> PointerType::children() const { return {_pointee_type}; }

// TODO: find out the pointer size from llvm::TargetMachine
int PointerType::get_align_bits() { return 64; }
int PointerType::get_size_bits() { return 64; }
int ArrayType::get_align_bits() { return 64; }
int ArrayType::get_size_bits() { return 64; }
int StringType::get_align_bits() { return 64; }
int StringType::get_size_bits() { return 64; }

ArrayType::ArrayType(Type *element_type, int size) : _element_type(element_type), _size(size) {
  _type_name = element_type->get_typename() + "[" + std::to_string(size) + "]";
}

vector<Type *> ArrayType::children() const { return {_element_type}; }

StringType::StringType() { _type_name = "str"; }

StructType::StructType(const str &name, const vector<Type *> &member_types) {
  _type_name = name;
  _member_types = member_types;
}

int StructType::get_align_bits() {
  int ret = 0;
  for (auto *t : _member_types) {
    ret = std::max(t->get_align_bits(), ret);
  }
  TAN_ASSERT(ret);
  return ret;
}

int StructType::get_size_bits() {
  // TODO: calculate struct size in bits
  return 8;
}

TypeRef::TypeRef(const str &name) { _type_name = name; }

FunctionType::FunctionType(Type *ret_type, const vector<Type *> &arg_types) {
  _ret_type = ret_type;
  _arg_types = arg_types;
}

Type *FunctionType::get_return_type() const { return _ret_type; }

vector<Type *> FunctionType::get_arg_types() const { return _arg_types; }

void FunctionType::set_arg_types(const vector<Type *> &arg_types) { _arg_types = arg_types; }

void FunctionType::set_return_type(Type *t) { _ret_type = t; }

vector<Type *> FunctionType::children() const {
  vector<Type *> ret{_ret_type};
  ret.insert(ret.begin(), _arg_types.begin(), _arg_types.end());
  return ret;
}