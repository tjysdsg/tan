#include "parser.h"
#include "src/ast/ast_statement.h"
#include "src/ast/ast_program.h"

namespace tanlang {

ASTProgram::ASTProgram() : ASTNode(tanlang::ASTType::PROGRAM,
    tanlang::op_precedence[tanlang::ASTType::PROGRAM],
    0,
    nullptr,
    0) {}

llvm::Value *ASTProgram::codegen(tanlang::CompilerSession *compiler_session) {
  for (const auto &e : _children) {
    e->codegen(compiler_session);
  }
  return nullptr;
}

size_t ASTProgram::nud(Parser *parser) {
  _end_index = _start_index;
  while (!parser->eof(_end_index)) {
    _children.push_back(parser->parse<tanlang::ASTType::STATEMENT>(_end_index, true));
  }
  return _end_index;
}

} // namespace tanlang
