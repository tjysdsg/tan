#include "src/ast/ast_expr.h"
#include "token.h"
#include "parser.h"

namespace tanlang {

Value *ASTParenthesis::codegen(ParserContext *parser_context) {
  auto *result = _children[0]->codegen(parser_context);
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) {
    _children[i]->codegen(parser_context);
  }
  return result;
}

}
