#include "src/ast/typed.h"

using namespace tanlang;

ASTTypePtr Typed::get_type() const { return _type; }

void Typed::set_type(const ASTTypePtr &type) { _type = type; }
