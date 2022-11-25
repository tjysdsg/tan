#include "ast/ast_named.h"

using namespace tanlang;

void ASTNamed::set_name(const str &name) { _name = name; }

str ASTNamed::get_name() const { return _name; }
