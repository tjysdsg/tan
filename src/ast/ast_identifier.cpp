#include <include/parser.h>
#include "src/ast/ast_identifier.h"
namespace tanlang {

void ASTIdentifier::nud(Parser *parser) {
  UNUSED(parser);
}

Value *ASTIdentifier::codegen(ParserContext *parser_context) {
  auto *v = parser_context->get_variable(_name);
  if (!v) {
    return nullptr;
  }
  return v;
}

}
