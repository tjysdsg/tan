#ifndef __TAN_SRC_AST_TYPED_H__
#define __TAN_SRC_AST_TYPED_H__
#include "base.h"

namespace tanlang {

AST_FWD_DECL(ASTType);

class Typed {
public:
  ASTTypePtr get_type() const;
  void set_type(const ASTTypePtr &type);

private:
  ASTTypePtr _type = nullptr;
};

}

#endif //__TAN_SRC_AST_TYPED_H__
