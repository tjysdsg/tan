#include "src/ast/ast_arg_decl.h"
#include "src/ast/ast_ty.h"

using namespace tanlang;

size_t ASTArgDecl::nud() {
  _end_index = _start_index;
  auto ret = _nud();
  if (!_is_type_resolved) { error("Must specify a type for the argument"); }
  return ret;
}
