#include "src/ast/ast_control_flow.h"
#include "src/ast/type_system.h"
#include "parser.h"
#include "intrinsic.h"

namespace tanlang {

Value *ASTIf::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  Value *condition = _children[0]->codegen(compiler_session);
  if (!condition) {
    auto *condition_token = _children[0]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }

  /// convert to bool if not
  condition = convert_to(compiler_session, compiler_session->get_builder()->getInt1Ty(), condition, false);

  Function *func = compiler_session->get_builder()->GetInsertBlock()->getParent();
  /// Create blocks for the then (and else) clause. Insert the 'then' block at the end of the function.
  BasicBlock *then_bb = BasicBlock::Create(*compiler_session->get_context(), "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*compiler_session->get_context(), "else");
  BasicBlock *merge_bb = BasicBlock::Create(*compiler_session->get_context(), "fi");

  compiler_session->get_builder()->CreateCondBr(condition, then_bb, else_bb);
  /// emit then value
  compiler_session->get_builder()->SetInsertPoint(then_bb);
  /// insert noop in case empty statement
  compiler_session->get_builder()->CreateCall(Intrinsic::GetIntrinsic(IntrinsicType::NOOP, compiler_session));
  _children[1]->codegen(compiler_session);
  compiler_session->get_builder()->CreateBr(merge_bb);

  /// emit else block
  func->getBasicBlockList().push_back(else_bb);
  compiler_session->get_builder()->SetInsertPoint(else_bb);
  /// insert noop in case empty statement
  compiler_session->get_builder()->CreateCall(Intrinsic::GetIntrinsic(IntrinsicType::NOOP, compiler_session));
  if (_has_else) {
    _children[2]->codegen(compiler_session);
  }
  compiler_session->get_builder()->CreateBr(merge_bb);
  /// emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  compiler_session->get_builder()->SetInsertPoint(merge_bb);
  /// insert noop in case empty statement
  compiler_session->get_builder()->CreateCall(Intrinsic::GetIntrinsic(IntrinsicType::NOOP, compiler_session));
  return nullptr;
}

Value *ASTElse::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  return _children[0]->codegen(compiler_session);
}

} // namespace tanlang

