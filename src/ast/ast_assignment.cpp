#include "src/ast/ast_assignment.h"
#include "src/ast/ast_var_decl.h"
#include "src/type_system.h"
#include "compiler_session.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

// TODO: allow chained assignment

Value *ASTAssignment::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  /// codegen the rhs
  auto lhs = _children[0];
  auto rhs = _children[1];
  Value *from = rhs->codegen(cs);
  Value *to = lhs->codegen(cs);

  if (rhs->is_lvalue()) { from = cs->get_builder()->CreateLoad(from); }
  if (!lhs->is_lvalue()) { report_code_error(lhs->_token, "Value can only be assigned to lvalue"); }
  if (!to) { report_code_error(lhs->_token, "Invalid left-hand operand of the assignment"); }
  if (!from) { report_code_error(rhs->_token, "Invalid expression for right-hand operand of the assignment"); }

  /// to is lvalue
  from = TypeSystem::ConvertTo(cs, to->getType()->getContainedType(0), from, false, true);
  cs->get_builder()->CreateStore(from, to);
  return to;
}

ASTAssignment::ASTAssignment(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::ASSIGN;
  _lbp = op_precedence[_type];
}

size_t ASTAssignment::led(const ASTNodePtr &left) {
  _end_index = _start_index + 1; /// skip "="
  _children.push_back(left);
  _children.push_back(_parser->next_expression(_end_index, 0));

  /// special case for variable declaration
  auto lhs = left;
  if (lhs->_type == ASTType::ID) {
    TAN_ASSERT(lhs->is_named());
    lhs = _cs->get(left->get_name());
    TAN_ASSERT(lhs);
  }
  if (lhs->_type == ASTType::VAR_DECL) {
    auto var = ast_cast<ASTVarDecl>(lhs);
    TAN_ASSERT(var);
    if (!var->is_type_resolved()) { var->set_ty(_children[1]->get_ty()); }
  }

  return _end_index;
}

/// always convert to lhs
size_t ASTAssignment::get_dominant_idx() const {
  // TODO:check type
  return 0;
}

} // namespace tanlang
