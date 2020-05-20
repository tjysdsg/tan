#include "src/ast/ast_assignment.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_var_decl.h"
#include "src/ast/ast_ty.h"
#include "src/type_system.h"
#include "compiler_session.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

Value *ASTAssignment::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  /// _codegen the rhs
  auto lhs = _children[0];
  auto rhs = _children[1];
  Value *from = rhs->codegen(cs);
  Value *to = lhs->codegen(cs);
  if (!from) { rhs->error("Invalid expression for right-hand operand of the assignment"); }
  if (!to) { lhs->error("Invalid left-hand operand of the assignment"); }
  if (!lhs->is_lvalue()) { lhs->error("Value can only be assigned to lvalue"); }

  from = TypeSystem::ConvertTo(cs, from, rhs->get_ty(), lhs->get_ty());
  builder->CreateStore(from, to);
  _llvm_value = to;
  return to;
}

size_t ASTAssignment::led(const ASTNodePtr &left) {
}

} // namespace tanlang
