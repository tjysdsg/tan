#include "src/ast/ast_control_flow.h"
#include "src/type_system.h"
#include "intrinsic.h"
#include "parser.h"
#include "compiler_session.h"

namespace tanlang {

Value *ASTIf::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  Value *condition = _children[0]->codegen(cs);
  if (!condition) {
    auto *condition_token = _children[0]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }

  /// convert to bool if not
  condition = TypeSystem::ConvertTo(cs, cs->get_builder()->getInt1Ty(), condition, false);

  /// Create blocks for the then (and else) clause
  Function *func = cs->get_builder()->GetInsertBlock()->getParent();
  BasicBlock *then_bb = BasicBlock::Create(*cs->get_context(), "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*cs->get_context(), "else");
  BasicBlock *merge_bb = BasicBlock::Create(*cs->get_context(), "fi");

  cs->get_builder()->CreateCondBr(condition, then_bb, else_bb);
  /// emit then value
  cs->get_builder()->SetInsertPoint(then_bb);
  _children[1]->codegen(cs);
  /// create a br instruction if there is no terminator instruction at the end of this block
  if (!cs->get_builder()->GetInsertBlock()->back().isTerminator()) { cs->get_builder()->CreateBr(merge_bb); }

  /// emit else block
  func->getBasicBlockList().push_back(else_bb);
  cs->get_builder()->SetInsertPoint(else_bb);
  if (_has_else) { _children[2]->codegen(cs); }
  /// create a br instruction if there is no terminator instruction at the end of this block
  if (!cs->get_builder()->GetInsertBlock()->back().isTerminator()) { cs->get_builder()->CreateBr(merge_bb); }

  /// emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  cs->get_builder()->SetInsertPoint(merge_bb);
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
  _end_index = condition->parse(_parser, _cs);
  _children.push_back(condition);
  /// if clause
  auto if_clause = _parser->peek(_end_index, TokenType::PUNCTUATION, "{");
  _end_index = if_clause->parse(_parser, _cs);
  _children.push_back(if_clause);

  /// else clause, if any
  auto *token = _parser->at(_end_index);
  if (token->type == TokenType::KEYWORD && token->value == "else") {
    auto else_clause = _parser->peek(_end_index);
    _end_index = else_clause->parse(_parser, _cs);
    _children.push_back(else_clause);
    _has_else = true;
  }
  return _end_index;
}

ASTIf::ASTIf(Token *token, size_t token_index) : ASTNode(ASTType::IF,
    op_precedence[ASTType::IF],
    0,
    token,
    token_index) {}

size_t ASTElse::nud() {
  _end_index = _start_index + 1; /// skip "else"
  auto else_clause = _parser->peek(_end_index);
  _end_index = else_clause->parse(_parser, _cs);
  _children.push_back(else_clause);
  return _end_index;
}

ASTElse::ASTElse(Token *token, size_t token_index) : ASTNode(ASTType::ELSE,
    op_precedence[ASTType::ELSE],
    0,
    token,
    token_index) {}

} // namespace tanlang

