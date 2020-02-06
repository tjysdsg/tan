#include <include/parser.h>
#include "src/ast/ast_identifier.h"

namespace tanlang {

Value *ASTIdentifier::codegen(CompilerSession *parser_context) {
  auto *v = parser_context->get(_name);
  if (!v) {
    return nullptr;
  }
  return v;
}

}
