#ifndef __TAN_SRC_AST_TYPED_H__
#define __TAN_SRC_AST_TYPED_H__
#include "base.h"
#include "src/ast/fwd.h"

namespace tanlang {

class Typed {
public:
  ASTType *get_type() const;
  void set_type(ASTType *type);

private:
  ASTType *_type = nullptr;
};

}

#endif //__TAN_SRC_AST_TYPED_H__
