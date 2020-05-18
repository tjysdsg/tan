#include "src/ast/ast_loop.h"
#include "src/type_system.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

llvm::Value *ASTLoop::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  cs->set_current_loop(this->shared_from_this());
  if (_loop_type == ASTLoopType::WHILE) {
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
    _loop_start = BasicBlock::Create(*cs->get_context(), "loop", func);
    BasicBlock *loop_body = BasicBlock::Create(*cs->get_context(), "loop_body", func);
    _loop_end = BasicBlock::Create(*cs->get_context(), "after_loop", func);

    /// start loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(_loop_start); }

    /// condition
    builder->SetInsertPoint(_loop_start);
    auto *cond = _children[0]->codegen(cs);
    if (!cond) { error("Expected a condition expression"); }
    cond = TypeSystem::ConvertTo(cs, cond, _children[0]->get_ty(), ASTTy::Create(Ty::BOOL));
    builder->CreateCondBr(cond, loop_body, _loop_end);

    /// loop body
    builder->SetInsertPoint(loop_body);
    _children[1]->codegen(cs);

    /// go back to the start of the loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(_loop_start); }

    /// end loop
    builder->SetInsertPoint(_loop_end);
  } else { TAN_ASSERT(false); }
  cs->set_current_loop(nullptr);
  return nullptr;
}

size_t ASTLoop::nud() {
  if (_parser->at(_start_index)->value == "for") {
    // TODO: implement for loop
    _loop_type = ASTLoopType::FOR;
    TAN_ASSERT(false);
  } else if (_parser->at(_start_index)->value == "while") {
    _loop_type = ASTLoopType::WHILE;
  } else {
    TAN_ASSERT(false);
  }
  _end_index = _start_index + 1; /// skip 'while'/'for'/...
  switch (_loop_type) {
    case ASTLoopType::WHILE:
      _parser->peek(_end_index, TokenType::PUNCTUATION, "(");
      _children.push_back(_parser->next_expression(_end_index)); /// condition
      _parser->peek(_end_index, TokenType::PUNCTUATION, "{");
      _children.push_back(_parser->next_expression(_end_index)); /// loop body
      break;
    case ASTLoopType::FOR:
      TAN_ASSERT(false);
      break;
    default:
      break;
  }
  return _end_index;
}

ASTLoop::ASTLoop(Token *token, size_t token_index) : ASTNode(ASTType::LOOP, 0, 0, token, token_index) {}

BasicBlock *ASTLoop::get_loop_end() const { return _loop_end; }

BasicBlock *ASTLoop::get_loop_start() const { return _loop_start; }

} // namespace tanlang


