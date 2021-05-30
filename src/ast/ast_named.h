#ifndef __TAN_SRC_AST_AST_NAMED_H__
#define __TAN_SRC_AST_AST_NAMED_H__
#include "base.h"

namespace tanlang {
class ASTNamed {
public:
  str get_name() const;
  void set_name(str_view name);

private:
  str _name;
};
}

#endif //__TAN_SRC_AST_AST_NAMED_H__
