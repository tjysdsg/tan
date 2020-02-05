#include "src/ast/ast_expr.h"
#include "token.h"
#include "common.h"
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

Value *ASTVarDecl::codegen(ParserContext *parser_context) {
  std::string name = std::reinterpret_pointer_cast<ASTIdentifier>(_children[0])->_name;
  std::string type_name = std::reinterpret_pointer_cast<ASTTypeName>(_children[1])->_name;
  Type *type = typename_to_llvm_type(type_name, parser_context);
  Value *var = create_block_alloca(parser_context->_builder->GetInsertBlock(), type, name);

  // add to current scope
  parser_context->add_variable(name, var);

  // set initial value
  if (_has_initial_val) {
    parser_context->_builder->CreateStore(_children[2]->codegen(parser_context), var);
  }
  return nullptr;
}

/// \attention UNUSED
Value *ASTArgDecl::codegen(ParserContext *parser_context) {
  UNUSED(parser_context);
  assert(false);
}

}
