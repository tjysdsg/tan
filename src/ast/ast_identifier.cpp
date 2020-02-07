#include <include/parser.h>
#include "src/ast/ast_identifier.h"

namespace tanlang {

Value *ASTIdentifier::codegen(CompilerSession *parser_context) {
  if (_op == ASTType::FUNC_CALL) {
    // Look up the name in the global module table.
    Function *func = parser_context->get_module()->getFunction(_name);
    if (!func) {
      throw std::runtime_error("Invalid function call");
    }

    // If argument mismatch error.
    if (func->arg_size() != _children.size()) {
      throw std::runtime_error("Invalid arguments");
    }

    std::vector<Value *> args_value;
    size_t n_args = _children.size();
    for (size_t i = 0; i < n_args; ++i) {
      args_value.push_back(_children[i]->codegen(parser_context));
      if (!args_value.back()) {
        return nullptr;
      }
    }
    return parser_context->get_builder()->CreateCall(func, args_value, "calltmp");
  }
  auto *v = parser_context->get(_name);
  if (!v) {
    return nullptr;
  }
  return v;
}

}
