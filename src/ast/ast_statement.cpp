#include "src/ast/ast_statement.h"
#include "parser.h"

namespace tanlang {

ASTStatement::ASTStatement(bool is_compound, Token *token, size_t token_index) : ASTNode(ASTType::STATEMENT,
                                                                                         op_precedence[ASTType::STATEMENT],
                                                                                         0,
                                                                                         token,
                                                                                         token_index
) {
  _is_compound = is_compound;
}

ASTStatement::ASTStatement(Token *token, size_t token_index) : ASTNode(ASTType::STATEMENT,
                                                                       op_precedence[ASTType::STATEMENT],
                                                                       0,
                                                                       token,
                                                                       token_index
) {}

ASTProgram::ASTProgram() : ASTNode(ASTType::PROGRAM, op_precedence[ASTType::PROGRAM], 0, nullptr, 0) {}

Value *ASTProgram::codegen(CompilerSession *compiler_session) {
  for (const auto &e : _children) {
    e->codegen(compiler_session);
  }
  return nullptr;
}

Value *ASTReturn::codegen(CompilerSession *compiler_session) {
  auto *result = _children[0]->codegen(compiler_session);
  /// create load if children is a pointer (because created by alloca)
  if (result->getType()->isPointerTy()) {
    result = compiler_session->get_builder()->CreateLoad(result, "load");
  }
  return compiler_session->get_builder()->CreateRet(result);
}

} // namespace tanlang
