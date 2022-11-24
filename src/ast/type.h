#ifndef __TAN_SRC_AST_TYPE_H__
#define __TAN_SRC_AST_TYPE_H__
#include "src/ast/ast_base.h"
#include "src/ast/fwd.h"

namespace tanlang {

class StringType;
class PrimitiveType;
class PointerType;
class ArrayType;
class TypeRef;
class StructType;

// TODO IMPORTANT: remove dependency from ASTBase
class Type : public ASTBase {
public:
  // TODO IMPORTANT: avoid duplicated instantiation
  [[nodiscard]] static PrimitiveType *GetVoidType(SrcLoc loc);
  [[nodiscard]] static PrimitiveType *GetBoolType(SrcLoc loc);
  [[nodiscard]] static PrimitiveType *GetCharType(SrcLoc loc);
  [[nodiscard]] static PrimitiveType *GetIntegerType(SrcLoc loc, size_t bit_size, bool is_unsigned);
  [[nodiscard]] static PrimitiveType *GetFloatType(SrcLoc loc, size_t bit_size);

  [[nodiscard]] static StringType *GetStringType(SrcLoc loc);
  [[nodiscard]] static PointerType *GetPointerType(SrcLoc loc, Type *pointee);
  [[nodiscard]] static ArrayType *GetArrayType(SrcLoc loc, Type *element_type, int size);

public:
  Type(SrcLoc loc) : ASTBase(ASTNodeType::TY, loc, 0) {};

  virtual bool is_primitive();
  virtual bool is_pointer();
  virtual bool is_array();
  virtual bool is_string();
  virtual bool is_struct();
  virtual bool is_ref();
  virtual bool is_float();
  virtual bool is_int();
  virtual bool is_unsigned();
  virtual bool is_bool();
  virtual bool is_void();
  virtual bool is_char();

  const str &get_typename() { return _type_name; }
  Type *get_canonical();
  bool is_canonical();
  void set_canonical(Type *t);

protected:
  str _type_name{};

private:
  Type *_canonical_type = nullptr;
};

class PrimitiveType : public Type {
public:
  enum Kind {
    VOID, CHAR, I8, I16, I32, I64, U8, U16, U32, U64, BOOL, F32, F64,
  };
  static inline umap<Kind, int> SIZE_BITS
      {{VOID, 0}, {CHAR, 8}, {I8, 8}, {I16, 16}, {I32, 32}, {I64, 64}, {U8, 8}, {U16, 16}, {U32, 32}, {U64, 64},
          {F32, 32}, {F64, 64}};
  static inline umap<Kind, str> TYPE_NAMES
      {{VOID, "void"}, {CHAR, "char"}, {I8, "i8"}, {I16, "i16"}, {I32, "i32"}, {I64, "i64"}, {U8, "u8"}, {U16, "u16"},
          {U32, "u32"}, {U64, "u64"}, {F32, "f32"}, {F64, "f64"}};
  static const inline umap<str, Kind> TYPENAME_TO_KIND =
      {{"int", I32}, {"i8", I8}, {"u8", U8}, {"i16", I16}, {"u16", U16}, {"i32", I32}, {"u32", U32}, {"i64", I64},
          {"u64", U64}, {"float", F32}, {"f32", F32}, {"f64", F64}, {"void", VOID}, {"char", CHAR}, {"bool", BOOL}};

public:
  [[nodiscard]] static PrimitiveType *Create(SrcLoc loc, Kind kind);

  bool is_primitive() override { return true; }

protected:
  PrimitiveType(SrcLoc loc) : Type(loc) {}

private:
  Kind _kind;
  Constructor *_constructor = nullptr;

  bool is_float() override { return _kind == F32 || _kind == F64; }
  bool is_int() override { return _kind >= I8 && _kind <= U64; }
  bool is_unsigned() override { return _kind >= U8 && _kind <= U64; };
  bool is_bool() override { return _kind == BOOL; }
  bool is_void() override { return _kind == VOID; }
  bool is_char() override { return _kind == CHAR; }

  int get_size_bits();
  int get_align_bits();
};

class PointerType : public Type {
public:
  bool is_pointer() { return true; }
  Type *get_pointee() { return _pointee_type; }

  friend class Type;

protected:
  PointerType(SrcLoc loc, Type *pointee_type);

private:
  Type *_pointee_type = nullptr;
};

class ArrayType : public Type {
public:
  friend class Type;

protected:
  ArrayType(SrcLoc loc, Type *element_type, int size);

private:
  Type *_element_type = nullptr;
  int _size = 0;
};

class StringType : public Type {
public:
  bool is_string() override { return true; }

  friend class Type;

protected:
  StringType(SrcLoc loc);
};

class StructType : public Type {
};

class TypeRef : public Type {
public:
  TypeRef(SrcLoc loc, str name);
  bool is_ref() { return true; }
};

// TODO: FunctionType

}

#endif //__TAN_SRC_AST_TYPE_H__
