#include "src/ast/typed.h"
#include "src/ast/type.h"

using namespace tanlang;

Type *Typed::get_type() const { return _type->get_canonical(); }

void Typed::set_type(Type *type) { _type = type; }
