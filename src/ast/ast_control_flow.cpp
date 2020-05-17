#include "src/ast/ast_control_flow.h"
#include "src/ast/ast_loop.h"
#include "src/type_system.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

Value *ASTIf::codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  Value *condition = _children[0]->codegen(cs);
  if (!condition) {
    auto *condition_token = _children[0]->_token;
    report_code_error(condition_token, "Invalid condition expression " + condition_token->to_string());
  }

  /// convert to bool if not
  condition = TypeSystem::ConvertTo(cs, builder->getInt1Ty(), condition, false);

  /// Create blocks for the then (and else) clause
  Function *func = builder->GetInsertBlock()->getParent();
  BasicBlock *then_bb = BasicBlock::Create(*cs->get_context(), "then", func);
  BasicBlock *else_bb = BasicBlock::Create(*cs->get_context(), "else");
  BasicBlock *merge_bb = BasicBlock::Create(*cs->get_context(), "fi");

  if (_has_else) { builder->CreateCondBr(condition, then_bb, else_bb); }
  else { builder->CreateCondBr(condition, then_bb, merge_bb); }
  /// emit then value
  builder->SetInsertPoint(then_bb);
  _children[1]->codegen(cs);
  /// create a br instruction if there is no terminator instruction at the end of then
  if (!builder->GetInsertBlock()->back().isTerminator()) { builder->CreateBr(merge_bb); }
  builder->SetInsertPoint(then_bb);
  if (!then_bb->back().isTerminator()) { builder->CreateBr(merge_bb); }

  /// emit else block
  if (_has_else) {
    func->getBasicBlockList().push_back(else_bb);
    builder->SetInsertPoint(else_bb);
    _children[2]->codegen(cs);
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

Value *ASTElse::codegen(CompilerSession *cs) {
  cs->set_current_debug_location(_token->l, _token->c);
  return _children[0]->codegen(cs);
}

llvm::Value *ASTBreakContinue::codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  auto loop = cs->get_current_loop();
  if (!loop) { report_code_error(_token, "Any break/continue statement must be inside loop"); }
  auto s = loop->get_loop_start();
  auto e = loop->get_loop_end();
  if (_type == ASTType::BREAK) {
    builder->CreateBr(e);
  } else if (_type == ASTType::CONTINUE) {
    builder->CreateBr(s);
  } else { TAN_ASSERT(false); }
  return nullptr;
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

size_t ASTElse::nud() {
  _end_index = _start_index + 1; /// skip "else"
  auto else_clause = _parser->peek(_end_index);
  _end_index = else_clause->parse(_parser, _cs);
  _children.push_back(else_clause);
  return _end_index;
}

size_t ASTBreakContinue::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

ASTIf::ASTIf(Token *t, size_t ti) : ASTNode(ASTType::IF, op_precedence[ASTType::IF], 0, t, ti) {}

ASTElse::ASTElse(Token *t, size_t ti) : ASTNode(ASTType::ELSE, op_precedence[ASTType::ELSE], 0, t, ti) {}

ASTBreakContinue::ASTBreakContinue(Token *t, size_t ti) : ASTNode(ASTType::INVALID, 0, 0, t, ti) {
  if (t->value == "break") { _type = ASTType::BREAK; }
  else if (t->value == "continue") { _type = ASTType::CONTINUE; }
}

} // namespace tanlang

