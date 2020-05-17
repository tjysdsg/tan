#include "src/ast/ast_assignment.h"
#include "src/ast/ast_var_decl.h"
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

  if (rhs->is_lvalue()) { from = builder->CreateLoad(from); }
  if (!lhs->is_lvalue()) { error("Value can only be assigned to lvalue"); }
  if (!to) { error("Invalid left-hand operand of the assignment"); }
  if (!from) { error("Invalid expression for right-hand operand of the assignment"); }

  /// to is lvalue
  from = TypeSystem::ConvertTo(cs, to->getType()->getContainedType(0), from, false, true);
  builder->CreateStore(from, to);
  _llvm_value = to;
  return to;
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

  _ty = _children[0]->get_ty();
  return _end_index;
}

bool ASTAssignment::is_lvalue() const { return true; }

ASTAssignment::ASTAssignment(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::ASSIGN;
  _lbp = op_precedence[_type];
}

} // namespace tanlang
