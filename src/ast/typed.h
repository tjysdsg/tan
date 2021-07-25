#ifndef __TAN_SRC_AST_TYPED_H__
#define __TAN_SRC_AST_TYPED_H__
#include "base.h"
#include "src/ast/fwd.h"

namespace tanlang {

class TypeAccessor {
public:
  virtual ASTType *get_type() const;
  virtual ~TypeAccessor() = default;
};

class Typed : public TypeAccessor {
public:
  ASTType *get_type() const override;
  virtual void set_type(ASTType *type);

private:
  ASTType *_type = nullptr;
};

}

#endif //__TAN_SRC_AST_TYPED_H__
