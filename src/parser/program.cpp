#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

/**
 * \brief: parse a list of (compound) statements
 * */
void ASTProgram::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  while(parser->_curr_token < n_tokens) {
    _children.push_back(parser->next_node());
  }
}

}
