#include "src/ast/ast_identifier.h"
#include <include/parser.h>

namespace tanlang {

Value *ASTIdentifier::codegen(CompilerSession *compiler_session) {
  auto var = std::reinterpret_pointer_cast<ASTVarDecl>(compiler_session->get(_name));
  auto *v = var->_llvm_value;
  if (!v) {
    return nullptr;
  }
  return v;
}

} // namespace tanlang
