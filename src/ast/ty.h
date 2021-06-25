#ifndef __TAN_SRC_AST_TY_H__
#define __TAN_SRC_AST_TY_H__

namespace tanlang {

#undef VOID
#undef CONST
enum class Ty : uint64_t {
  INVALID = 0,
  /// basic types 1->12 bits
  #define TY_BASE_MASK 0xfffu
  VOID = 1u,
  INT = 2u,
  FLOAT = 3u,
  DOUBLE = 4u,
  BOOL = 5u,
  POINTER = 6u,
  STRING = 7u,
  CHAR = 8u,
  FUNC_PTR = 9u, // TODO: function ptr
  STRUCT = 10u,
  ARRAY = 11u,
  ENUM = 12u,
  TYPE_REF = 13u,

  /// qualifiers 13->32 bits
  #define TY_QUALIFIER_MASK 0xffffff000u

  UNSIGNED = 1u << 13u,
  CONST = 1u << 14u,
  BIT8 = 1u << 15u,
  BIT16 = 1u << 16u,
  BIT32 = 1u << 17u,
  BIT64 = 1u << 18u,
};

}

#endif //__TAN_SRC_AST_TY_H__
