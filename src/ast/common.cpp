#include "src/ast/common.h"

namespace tanlang {

AllocaInst *CreateEntryBlockAlloca(Function *func, const std::string &name, ParserContext *parser_context) {
  IRBuilder<> tmp_builder(&func->getEntryBlock(), func->getEntryBlock().begin());
  return tmp_builder.CreateAlloca(parser_context->_builder->getFloatTy(), nullptr, name);
}

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list) {
  bool r = false;
  for (const auto elem : list) {
    if (t == elem) {
      r = true;
    }
  }
  return r;
}

}
