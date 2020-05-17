#include "parser.h"
#include "src/ast/ast_statement.h"
#include "src/ast/ast_program.h"

namespace tanlang {

ASTProgram::ASTProgram() : ASTNode(ASTType::PROGRAM, op_precedence[tanlang::ASTType::PROGRAM], 0, nullptr, 0) {}

llvm::Value *ASTProgram::_codegen(CompilerSession *cs) {
  for (const auto &e : _children) { e->codegen(cs); }
  return nullptr;
}

size_t ASTProgram::nud() {
  _end_index = _start_index;
  while (!_parser->eof(_end_index)) {
    _children.push_back(_parser->parse<tanlang::ASTType::STATEMENT>(_end_index, true));
  }
  return _end_index;
}

} // namespace tanlang
