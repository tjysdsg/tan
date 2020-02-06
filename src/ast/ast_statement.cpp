#include "parser.h"
#include "src/ast/ast_statement.h"
#include "src/ast/astnode.h"
#include "src/llvm_include.h"

namespace tanlang {

ASTStatement::ASTStatement(bool is_compound, Token *token) : ASTNode(ASTType::STATEMENT,
                                                                     op_precedence[ASTType::STATEMENT],
                                                                     0,
                                                                     token) {
  _is_compound = is_compound;
}

ASTStatement::ASTStatement(Token *token) : ASTNode(ASTType::STATEMENT,
                                                   op_precedence[ASTType::STATEMENT],
                                                   0,
                                                   token) {}

ASTProgram::ASTProgram() : ASTNode(ASTType::PROGRAM, op_precedence[ASTType::PROGRAM], 0, nullptr) {}

Value *ASTProgram::codegen(CompilerSession *parser_context) {
  for (const auto &e : _children) {
    e->codegen(parser_context);
  }
  return nullptr;
}

Value *ASTReturn::codegen(CompilerSession *parser_context) {
  return parser_context->_builder->CreateRet(_children[0]->codegen(parser_context));
}

} // namespace tanlang
