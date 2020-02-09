#include "src/ast/ast_control_flow.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTIf::codegen(CompilerSession *compiler_session) {
  Value *condition = _children[0]->codegen(compiler_session);
  if (!condition) {
    auto *condition_token = _children[0]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }

  // convert condition to a bool by comparing non-equal to 0
  /* condition = parser_context->_builder->CreateICmpEQ(condition,
                                                     ConstantInt::get(*parser_context->_context, APInt(32, 0)),
                                                     "if");
                                                     */
  Function *func = compiler_session->get_builder()->GetInsertBlock()->getParent();

  // Create blocks for the then (and else) clause. Insert the 'then' block at the end of the function.
  BasicBlock *then_bb = BasicBlock::Create(*compiler_session->get_context(), "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*compiler_session->get_context(), "else");
  BasicBlock *merge_bb = BasicBlock::Create(*compiler_session->get_context(), "fi");

  compiler_session->get_builder()->CreateCondBr(condition, then_bb, else_bb);
  // Emit then value
  compiler_session->get_builder()->SetInsertPoint(then_bb);
  Value *then = _children[1]->codegen(compiler_session);
  if (!then) {
    auto *condition_token = _children[1]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }
  compiler_session->get_builder()->CreateBr(merge_bb);
  // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
  then_bb = compiler_session->get_builder()->GetInsertBlock();

  Value *else_ = nullptr;
  if (_has_else) {
    // Emit else block
    func->getBasicBlockList().push_back(else_bb);
    compiler_session->get_builder()->SetInsertPoint(else_bb);

    else_ = _children[2]->codegen(compiler_session);
    if (!else_) {
      auto *condition_token = _children[2]->_token;
      report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
    }
    compiler_session->get_builder()->CreateBr(merge_bb);
    // codegen of 'Else' can change the current block, update ElseBB for the PHI.
    else_bb = compiler_session->get_builder()->GetInsertBlock();
  }
  // Emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  compiler_session->get_builder()->SetInsertPoint(merge_bb);
  return nullptr;
}

}

