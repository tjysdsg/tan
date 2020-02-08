#include "src/ast/ast_identifier.h"
#include <include/parser.h>

namespace tanlang {

Value *ASTIdentifier::codegen(CompilerSession *compiler_session) {
  auto *v = compiler_session->get(_name);
  if (!v) {
    return nullptr;
  }
  return v;
}

} // namespace tanlang
