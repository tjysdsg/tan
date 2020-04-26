#include "ast_arg_decl.h"

namespace tanlang {

ASTArgDecl::ASTArgDecl(Token *token, size_t token_index) : ASTVarDecl(token, token_index) {
  _type = ASTType::ARG_DECL;
}

size_t ASTArgDecl::nud(Parser *parser) {
  _end_index = _start_index;
  return _nud(parser);
}

} // namespace tanlang

