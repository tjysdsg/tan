#include "src/ast/ast_ty.h"
#include "src/ast/ast_loop.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

namespace tanlang {

llvm::Value *ASTBreakContinue::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  auto loop = cs->get_current_loop();
  if (!loop) { error("Any break/continue statement must be inside loop"); }
  auto s = loop->get_loop_start();
  auto e = loop->get_loop_end();
  if (_type == ASTType::BREAK) {
    builder->CreateBr(e);
  } else if (_type == ASTType::CONTINUE) {
    builder->CreateBr(s);
  } else { TAN_ASSERT(false); }
  return nullptr;
}

size_t ASTBreakContinue::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

ASTBreakContinue::ASTBreakContinue(Token *t, size_t ti) : ASTNode(ASTType::INVALID, 0, 0, t, ti) {
  if (t->value == "break") { _type = ASTType::BREAK; }
  else if (t->value == "continue") { _type = ASTType::CONTINUE; }
}

} // namespace tanlang

