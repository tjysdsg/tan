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
/**
 * \brief Type is immutable once created, and it's made sure that each type has only one instance
 */
class Type : public ASTBase {
public:
  // TODO IMPORTANT: avoid duplicated instantiation
  [[nodiscard]] static PrimitiveType *GetVoidType();
  [[nodiscard]] static PrimitiveType *GetBoolType();
  [[nodiscard]] static PrimitiveType *GetCharType();
  [[nodiscard]] static PrimitiveType *GetIntegerType(size_t bit_size, bool is_unsigned);
  [[nodiscard]] static PrimitiveType *GetFloatType(size_t bit_size);

  [[nodiscard]] static StringType *GetStringType();
  [[nodiscard]] static PointerType *GetPointerType(Type *pointee);
  [[nodiscard]] static ArrayType *GetArrayType(Type *element_type, int size);

public:
  Type() : ASTBase(ASTNodeType::TY, SrcLoc(0), 0) {};

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
  virtual bool is_enum();

  const str &get_typename() { return _type_name; }
  Type *get_canonical() const;
  void set_canonical(Type *t);

protected:
  str _type_name{};

private:
  Type *_canonical_type = nullptr;

  // type cache
  static StringType *STRING_TYPE;
  static inline umap<Type *, PointerType *> POINTER_TYPE_CACHE{}; // pointee type -> pointer type
  static inline umap<std::pair<Type *, int>, ArrayType *, PairHash>
      ARRAY_TYPE_CACHE{}; // (element type, size) -> array type
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
  [[nodiscard]] static PrimitiveType *Create(Kind kind);

  bool is_primitive() override { return true; }
  bool is_float() override { return _kind == F32 || _kind == F64; }
  bool is_int() override { return _kind >= I8 && _kind <= U64; }
  bool is_unsigned() override { return _kind >= U8 && _kind <= U64; };
  bool is_bool() override { return _kind == BOOL; }
  bool is_void() override { return _kind == VOID; }
  bool is_char() override { return _kind == CHAR; }

  int get_align_bits();
  int get_size_bits();

protected:
  PrimitiveType() = default;

private:
  static inline umap<PrimitiveType::Kind, PrimitiveType *> CACHE{};

  Kind _kind;
};

class PointerType : public Type {
public:
  bool is_pointer() { return true; }
  Type *get_pointee() { return _pointee_type; }

  friend class Type;

protected:
  PointerType(Type *pointee_type);

private:
  Type *_pointee_type = nullptr;
};

class ArrayType : public Type {
public:
  Type *get_element_type() { return _element_type; }
  int get_size() { return _size; }
  bool is_array() override { return true; }

  friend class Type;

protected:
  ArrayType(Type *element_type, int size);

private:
  Type *_element_type = nullptr;
  int _size = 0;
};

class StringType : public Type {
public:
  bool is_string() override { return true; }

  friend class Type;

protected:
  StringType();
};

class StructDecl;
class StructType : public Type {
public:
  bool is_struct() override { return true; }

  friend class Type;

protected:
  StructType(const str &name, StructDecl *struct_decl) {
    _type_name = name;
  }
};

}

#endif //__TAN_SRC_AST_TYPE_H__
