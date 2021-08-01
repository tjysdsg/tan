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
  FLOAT = 3u,
  BOOL = 4u,
  POINTER = 5u,
  STRING = 6u,
  CHAR = 7u,
  STRUCT = 8u,
  ARRAY = 9u,
  ENUM = 10u,
  TYPE_REF = 11u,
  FUNC_PTR = 12u, // TODO: function ptr

  /// qualifiers 13->32 bits
  #define TY_QUALIFIER_MASK 0xffffff000u

  UNSIGNED = 1u << 13u,
  CONST = 1u << 14u,
};

}

#endif //__TAN_SRC_AST_TY_H__
