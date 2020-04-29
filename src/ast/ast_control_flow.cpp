#include "src/ast/ast_control_flow.h"
#include "src/type_system.h"
#include "intrinsic.h"
#include "parser.h"

namespace tanlang {

Value *ASTIf::codegen(CompilerSession *compiler_session) {
  compiler_session->set_current_debug_location(_token->l, _token->c);
  Value *condition = _children[0]->codegen(compiler_session);
  if (!condition) {
    auto *condition_token = _children[0]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }

  /// convert to bool if not
  condition = TypeSystem::ConvertTo(compiler_session, compiler_session->get_builder()->getInt1Ty(), condition, false);

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

size_t ASTIf::nud() {
  _end_index = _start_index + 1; /// skip "if"
  /// condition
  auto condition = _parser->peek(_end_index, TokenType::PUNCTUATION, "(");
  _end_index = condition->parse(_parser);
  _children.push_back(condition);
  /// if clause
  auto if_clause = _parser->peek(_end_index, TokenType::PUNCTUATION, "{");
  _end_index = if_clause->parse(_parser);
  _children.push_back(if_clause);

  /// else clause, if any
  auto *token = _parser->at(_end_index);
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    auto else_clause = _parser->peek(_end_index);
    _end_index = else_clause->parse(_parser);
    _children.push_back(else_clause);
    _has_else = true;
  }
  return _end_index;
}

size_t ASTElse::nud() {
  _end_index = _start_index + 1; /// skip "else"
  auto else_clause = _parser->peek(_end_index);
  _end_index = else_clause->parse(_parser);
  _children.push_back(else_clause);
  return _end_index;
}

} // namespace tanlang

