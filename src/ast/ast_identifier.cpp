#include <include/parser.h>
#include "src/ast/ast_identifier.h"
namespace tanlang {

Value *ASTIdentifier::codegen(ParserContext *parser_context) {
  auto *v = parser_context->get_variable(_name);
  if (!v) {
    return nullptr;
  }
  return v;
}

}
