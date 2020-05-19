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
  _end_index = _start_index + 1; /// skip "="
  _children.push_back(left);
  _children.push_back(_parser->next_expression(_end_index, 0));

  /// special case for variable declaration
  auto lhs = left;
  if (lhs->_type == ASTType::ID) {
    TAN_ASSERT(lhs->is_named());
    auto id = ast_cast<ASTIdentifier>(lhs);
    lhs = id->get_referred();
  }
  if (lhs->_type == ASTType::VAR_DECL) {
    auto var = ast_cast<ASTVarDecl>(lhs);
    TAN_ASSERT(var);
    if (!var->is_type_resolved()) {
      auto ty = _children[1]->get_ty();
      ty = std::make_shared<ASTTy>(*ty); // copy
      ty->set_is_lvalue(true);
      var->set_ty(ty);
      _children[0] = var;
    }
  }

  _ty = _children[0]->get_ty();
  if (TypeSystem::CanImplicitCast(_ty, _children[1]->get_ty()) != 0) {
    error("Cannot perform implicit type conversion");
  }
  return _end_index;
}

bool ASTAssignment::is_lvalue() { return true; }

ASTAssignment::ASTAssignment(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::ASSIGN;
  _lbp = op_precedence[_type];
}

} // namespace tanlang
