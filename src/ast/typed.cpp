#include "src/ast/typed.h"

using namespace tanlang;

ASTType *TypeAccessor::get_type() const {
  TAN_ASSERT(false);
  return nullptr;
}

ASTType *Typed::get_type() const { return _type; }

void Typed::set_type(ASTType *type) { _type = type; }
