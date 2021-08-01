#ifndef __TAN_SRC_AST_TY_H__
#define __TAN_SRC_AST_TY_H__
#include "base.h"

namespace tanlang {

#undef VOID
#undef CONST
enum class Ty : uint64_t {
  INVALID = 0,
  /// basic types 1->12 bits
  #define TY_BASE_MASK 0xfffu
  VOID = 1u,
  INT = 2u,
  FLOAT = 3u,    // TODO: use bit_size to distinguish FLOAT and DOUBLE
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
};

}

#endif //__TAN_SRC_AST_TY_H__
