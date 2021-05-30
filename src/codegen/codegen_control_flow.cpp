#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "src/ast/ast_node.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/type_system.h"
#include "src/ast/factory.h"
#include "compiler_session.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_break_continue(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  auto loop = _cs->get_current_loop();
  if (!loop) {
    report_error(p, "Any break/continue statement must be inside loop");
  }
  auto s = loop->_loop_start;
  auto e = loop->_loop_end;
  if (p->get_node_type() == ASTNodeType::BREAK) {
    builder->CreateBr(e);
  } else if (p->get_node_type() == ASTNodeType::CONTINUE) {
    builder->CreateBr(s);
  } else { TAN_ASSERT(false); }
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_loop(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  auto prev_loop = _cs->get_current_loop();
  auto pl = ast_cast<ASTLoop>(p);
  TAN_ASSERT(pl);

  set_current_debug_location(p);
  _cs->set_current_loop(pl);
  if (pl->_loop_type == ASTLoopType::WHILE) {
    /*
     * Results should like this:
     *
     * ...
     * loop:
     *    exit condition check, goto 'loop_body' or 'after_loop'
     * loop_body:
     *    ...
     *    goto 'loop'
     * after_loop:
     *    ...
     * */

    Function *func = builder->GetInsertBlock()->getParent();

    /// make sure to set _loop_start and _loop_end before generating loop_body, cuz break and continue statements
    /// use these two (get_loop_start() and get_loop_end())
    pl->_loop_start = BasicBlock::Create(*_cs->get_context(), "loop", func);
    BasicBlock *loop_body = BasicBlock::Create(*_cs->get_context(), "loop_body", func);
    pl->_loop_end = BasicBlock::Create(*_cs->get_context(), "after_loop", func);

    /// start loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(pl->_loop_start); }

    /// condition
    builder->SetInsertPoint(pl->_loop_start);
    auto *cond = codegen(p->get_child_at<ASTNode>(0));
    if (!cond) {
      report_error(p, "Expected a condition expression");
    }
    cond = TypeSystem::ConvertTo(_cs, cond, p->get_child_at<ASTNode>(0)->_type, create_ty(_cs, Ty::BOOL));
    builder->CreateCondBr(cond, loop_body, pl->_loop_end);

    /// loop body
    builder->SetInsertPoint(loop_body);
    codegen(p->get_child_at<ASTNode>(1));

    /// go back to the start of the loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(pl->_loop_start); }

    /// end loop
    builder->SetInsertPoint(pl->_loop_end);
  } else { TAN_ASSERT(false); }
  _cs->set_current_loop(prev_loop); /// restore the outer loop
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_if(const ASTNodePtr &p) {
  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  Value *condition = codegen(p->get_child_at<ASTNode>(0));
  if (!condition) {
    report_error(p, "Invalid condition expression ");
  }

  /// convert to bool if not
  condition = TypeSystem::ConvertTo(_cs, condition, p->get_child_at<ASTNode>(0)->_type, create_ty(_cs, Ty::BOOL));

  /// create_ty blocks for the then (and else) clause
  Function *func = builder->GetInsertBlock()->getParent();
  BasicBlock *then_bb = BasicBlock::Create(*_cs->get_context(), "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*_cs->get_context(), "else");
  BasicBlock *merge_bb = BasicBlock::Create(*_cs->get_context(), "fi");

  auto pif = ast_cast<ASTIf>(p);
  TAN_ASSERT(pif);
  if (pif->_has_else) { builder->CreateCondBr(condition, then_bb, else_bb); }
  else { builder->CreateCondBr(condition, then_bb, merge_bb); }

  /// emit then value
  builder->SetInsertPoint(then_bb);
  codegen(p->get_child_at<ASTNode>(1));
  /// create a br instruction if there is no terminator instruction at the end of then
  if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(merge_bb); }
  builder->SetInsertPoint(then_bb);
  if (!then_bb->back().isTerminator()) { builder->CreateBr(merge_bb); }

  /// emit else block
  if (pif->_has_else) {
    func->getBasicBlockList().push_back(else_bb);
    builder->SetInsertPoint(else_bb);
    codegen(p->get_child_at<ASTNode>(2));
    /// create a br instruction if there is no terminator instruction at the end of else
    if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(merge_bb); }
    builder->SetInsertPoint(else_bb);
    if (!else_bb->back().isTerminator()) { builder->CreateBr(merge_bb); }
  }

  /// emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  builder->SetInsertPoint(merge_bb);
  return nullptr;
}
