#include "ast_import.h"

namespace tanlang {

ASTImport::ASTImport(Token *token, size_t token_index) : ASTNode(ASTType::IMPORT, 0, 0, token, token_index) {}

std::string ASTImport::to_string(bool print_prefix) const {
  std::string ret = "";
  if (print_prefix) {
    ret += ASTNode::to_string(print_prefix) + " ";
  }
  ret += _file;
  return ret;
}

Value *ASTImport::codegen(CompilerSession *compiler_session) {
  return ASTNode::codegen(compiler_session);
}

}
