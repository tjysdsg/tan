#include "src/codegen/code_generator_impl.h"
#include "src/llvm_include.h"
#include "src/ast/ast_type.h"
#include "src/ast/decl.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/analysis/type_system.h"
#include "compiler_session.h"

using namespace tanlang;

Value *CodeGeneratorImpl::codegen_break_continue(ASTBase *p) {
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
  } else {
    TAN_ASSERT(false);
  }
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_loop(ASTBase *_p) {
  auto p = ast_must_cast<Loop>(_p);

  auto *builder = _cs->_builder;
  auto prev_loop = _cs->get_current_loop();

  set_current_debug_location(p);
  _cs->set_current_loop(p);
  if (p->_loop_type == ASTLoopType::WHILE) {
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
    p->_loop_start = BasicBlock::Create(*_cs->get_context(), "loop", func);
    BasicBlock *loop_body = BasicBlock::Create(*_cs->get_context(), "loop_body", func);
    p->_loop_end = BasicBlock::Create(*_cs->get_context(), "after_loop", func);

    /// start loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!builder->GetInsertBlock()->back().isTerminator()) {
      builder->CreateBr(p->_loop_start);
    }

    /// condition
    builder->SetInsertPoint(p->_loop_start);
    auto *cond = codegen(p->get_predicate());
    if (!cond) {
      report_error(p, "Expected a condition expression");
    }
    cond = TypeSystem::ConvertTo(_cs, cond, p->get_predicate()->get_type(), ASTType::Create(_cs, Ty::BOOL));
    builder->CreateCondBr(cond, loop_body, p->_loop_end);

    /// loop body
    builder->SetInsertPoint(loop_body);
    codegen(p->get_body());

    /// go back to the start of the loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!builder->GetInsertBlock()->back().isTerminator()) {
      builder->CreateBr(p->_loop_start);
    }

    /// end loop
    builder->SetInsertPoint(p->_loop_end);
  } else {
    TAN_ASSERT(false);
  }

  _cs->set_current_loop(prev_loop); /// restore the outer loop
  return nullptr;
}

Value *CodeGeneratorImpl::codegen_if(ASTBase *_p) {
  auto p = ast_must_cast<If>(_p);

  auto *builder = _cs->_builder;
  set_current_debug_location(p);

  Function *func = builder->GetInsertBlock()->getParent();
  BasicBlock *merge_bb = BasicBlock::Create(*_cs->get_context(), "endif");
  size_t n = p->get_num_branches();

  /// create basic blocks
  vector<BasicBlock *> cond_blocks(n);
  vector<BasicBlock *> then_blocks(n);
  for (size_t i = 0; i < n; ++i) {
    cond_blocks[i] = BasicBlock::Create(*_cs->get_context(), "cond", func);
    then_blocks[i] = BasicBlock::Create(*_cs->get_context(), "branch", func);
  }

  /// codegen branches
  builder->CreateBr(cond_blocks[0]);
  for (size_t i = 0; i < n; ++i) {
    /// condition
    builder->SetInsertPoint(cond_blocks[i]);

    Expr *cond = p->get_predicate(i);
    if (!cond) { /// else clause, immediately go to then block
      TAN_ASSERT(i == n - 1); /// only the last branch can be an else
      builder->CreateBr(then_blocks[i]);
    } else {
      Value *cond_v = codegen(cond);
      if (!cond_v) { report_error(p, "Invalid condition expression "); }
      /// convert to bool
      cond_v = TypeSystem::ConvertTo(_cs, cond_v, cond->get_type(), ASTType::Create(_cs, Ty::BOOL));
      if (i < n - 1) {
        builder->CreateCondBr(cond_v, then_blocks[i], cond_blocks[i + 1]);
      } else {
        builder->CreateCondBr(cond_v, then_blocks[i], merge_bb);
      }
    }

    /// then clause
    builder->SetInsertPoint(then_blocks[i]);
    codegen(p->get_branch(i));

    /// go to merge block if there is no terminator instruction at the end of then
    if (!builder->GetInsertBlock()->back().isTerminator()) {
      builder->CreateBr(merge_bb);
    }
  }

  /// emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  builder->SetInsertPoint(merge_bb);
  return nullptr;
}

