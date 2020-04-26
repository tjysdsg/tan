#include "src/ast/ast_loop.h"
#include "src/ast/type_system.h"
#include "compiler_session.h"
#include "parser.h"
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

    /// end to loop start
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

size_t ASTLoop::nud(tanlang::Parser *parser) {
  if (parser->at(_start_index)->value == "for") {
    _loop_type = ASTLoopType::FOR;
    assert(false);
  } else if (parser->at(_start_index)->value == "while") {
    _loop_type = ASTLoopType::WHILE;
  } else {
    assert(false);
  }
  _end_index = _start_index + 1; /// skip 'while'/'for'/...
  switch (_loop_type) {
    case ASTLoopType::WHILE:
      parser->peek(_end_index, TokenType::PUNCTUATION, "(");
      _children.push_back(parser->next_expression(_end_index)); /// condition
      parser->peek(_end_index, TokenType::PUNCTUATION, "{");
      _children.push_back(parser->next_expression(_end_index)); /// loop body
      break;
    case ASTLoopType::FOR:
      assert(false);
      break;
    default:
      break;
  }
  return _end_index;
}

} // namespace tanlang


