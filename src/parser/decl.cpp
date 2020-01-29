#include "src/ast/ast_expr.h"
#include "parser.h"

namespace tanlang {

void ASTArgDef::nud(Parser *parser) {
  _children.push_back(parser->next_node()); // name
  parser->advance(TokenType::PUNCTUATION, ":");
  _children.push_back(parser->next_node()); // type
}

}
