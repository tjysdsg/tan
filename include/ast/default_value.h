#ifndef __TAN_SRC_AST_DEFAULT_VALUE_H__
#define __TAN_SRC_AST_DEFAULT_VALUE_H__

#include "expr.h"

namespace tanlang {

class DefaultValue {
public:
  static Literal *CreateTypeDefaultValueLiteral(TokenizedSourceFile *src, Type *type);
};

} // namespace tanlang

#endif //__TAN_SRC_AST_DEFAULT_VALUE_H__
