#include "ast/typed.h"
#include "ast/type.h"

using namespace tanlang;

Type *Typed::get_type() const {
  if (!_type) { return nullptr; }
  return _type;
}

void Typed::set_type(Type *type) {
  TAN_ASSERT(type);
  _type = type;
}
