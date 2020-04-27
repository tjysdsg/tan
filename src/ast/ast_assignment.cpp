#include "src/ast/ast_assignment.h"
#include "src/type_system.h"
#include "compiler_session.h"
#include "parser.h"

namespace tanlang {

Value *ASTAssignment::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  /// codegen the rhs
  auto lhs = _children[0];
  auto rhs = _children[1];
  Value *from = rhs->codegen(compiler_session);
  Value *to = lhs->codegen(compiler_session);

  if (rhs->is_lvalue()) {
    from = compiler_session->get_builder()->CreateLoad(from);
  }

  if (!lhs->is_lvalue()) {
    report_code_error(lhs->_token, "Value can only be assigned to lvalue");
  }

  if (!to) {
    report_code_error(lhs->_token, "Invalid left-hand operand of the assignment");
  }
  if (!from) {
    report_code_error(rhs->_token, "Invalid expression for right-hand operand of the assignment");
  }
  /// to is lvalue
  from = convert_to(compiler_session, to->getType()->getContainedType(0), from, false, true);
  compiler_session->get_builder()->CreateStore(from, to);
  return to;
}

ASTAssignment::ASTAssignment(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::ASSIGN;
  _lbp = op_precedence[_type];
}

size_t ASTAssignment::led(const ASTNodePtr &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip "="
  _children.push_back(left);
  _children.push_back(parser->next_expression(_end_index, 0));
  return _end_index;
}

/// always convert to lhs
size_t ASTAssignment::get_dominant_idx() const { return 0; }

} // namespace tanlang
