#include "ast/typed.h"
#include "ast/type.h"

using namespace tanlang;

Type *Typed::get_type() const { return _type; }

void Typed::set_type(Type *type) { _type = type; }
