#include "src/ast/ast_control_flow.h"
#include "parser.h"
#include "token.h"
#include "intrinsic.h"

namespace tanlang {

Value *ASTIf::codegen(CompilerSession *compiler_session) {
  Value *condition = _children[0]->codegen(compiler_session);
  if (!condition) {
    auto *condition_token = _children[0]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }

  /// convert to i32 if not
  unsigned condition_bits = condition->getType()->getPrimitiveSizeInBits();
  if (condition_bits != 1) {
    auto *op_type = compiler_session->get_builder()->getIntNTy(condition_bits);
    if (!condition->getType()->isIntegerTy()) { // bitcase to int if not
      condition = compiler_session->get_builder()->CreateBitCast(condition, op_type);
    }
    condition = compiler_session->get_builder()->CreateICmpNE(condition, ConstantInt::get(op_type, 0, true));
  }

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
  Value *then = _children[1]->codegen(compiler_session);
  if (!then) {
    auto *condition_token = _children[1]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }
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
  return nullptr;
}

Value *ASTElse::codegen(CompilerSession *compiler_session) {
  return _children[0]->codegen(compiler_session);
}

} // namespace tanlang

