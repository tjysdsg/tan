#include "src/ast/typed.h"

using namespace tanlang;

ASTType *Typed::get_type() const { return _type; }

void Typed::set_type(ASTType *type) { _type = type; }
