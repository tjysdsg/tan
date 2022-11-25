#ifndef __TAN_SRC_AST_TYPED_H__
#define __TAN_SRC_AST_TYPED_H__
#include "../base.h"
#include "fwd.h"

namespace tanlang {

/**
 * \brief All typed AST nodes should inherit this class
 */
class Typed {
public:
  virtual Type *get_type() const;
  virtual void set_type(Type *type);
  virtual ~Typed() = default;

private:
  Type *_type = nullptr;
};

}

#endif //__TAN_SRC_AST_TYPED_H__
