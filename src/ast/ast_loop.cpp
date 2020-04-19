#include "src/ast/ast_loop.h"
#include "src/ast/type_system.h"
#include "compiler_session.h"
#include "intrinsic.h"
#include "token.h"

namespace tanlang {

llvm::Value *ASTLoop::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  if (_loop_type == ASTLoopType::WHILE) {
    Function *func = compiler_session->get_builder()->GetInsertBlock()->getParent();
    BasicBlock *loop_bb = BasicBlock::Create(*compiler_session->get_context(), "loop", func);

    /// enter loop_bb
    compiler_session->get_builder()->CreateBr(loop_bb);

    /// loop body
    BasicBlock *body_bb = BasicBlock::Create(*compiler_session->get_context(), "loop_body", func);
    compiler_session->get_builder()->SetInsertPoint(body_bb);
    _children[1]->codegen(compiler_session);

    /// back to loop start
    compiler_session->get_builder()->CreateBr(loop_bb);

    /// after loop
    BasicBlock *after_bb = BasicBlock::Create(*compiler_session->get_context(), "after_loop", func);

    /// condition
    /// to make sure that the last code block is after_bb, after_bb is created after loop body is generated
    /// since the call to CreateCondBr should depends on after_bb, this is generated last
    compiler_session->get_builder()->SetInsertPoint(loop_bb);
    auto *cond = _children[0]->codegen(compiler_session);
    /// convert to bool if not
    cond = convert_to(compiler_session, compiler_session->get_builder()->getInt1Ty(), cond, false);

    compiler_session->get_builder()->CreateCondBr(cond, body_bb, after_bb);

    /// done
    compiler_session->get_builder()->SetInsertPoint(after_bb);
    compiler_session->get_builder()->CreateCall(Intrinsic::GetIntrinsic(IntrinsicType::NOOP, compiler_session));
  } else {
    assert(false);
  }
  return nullptr;
}

} // namespace tanlang


