#include "src/common.h"

namespace tanlang {

AllocaInst *create_block_alloca(BasicBlock *block, Type *type, size_t size, const str &name) {
  block = &block->getParent()->getEntryBlock();
  IRBuilder<> tmp_builder(block, block->begin());
  if (size <= 1) {
    return tmp_builder.CreateAlloca(type, nullptr, name);
  } else {
    return tmp_builder.CreateAlloca(type, tmp_builder.getInt32((unsigned) size), name);
  }
}

bool is_ast_type_in(ASTNodeType t, const vector<ASTNodeType> &list) {
  return std::find(list.begin(), list.end(), t) != list.end();
}

bool is_string_in(const str &s, const vector<str> &list) {
  return std::find(list.begin(), list.end(), s) != list.end();
}

} // namespace tanlang

#ifdef DEBUG
const char *__asan_default_options() { return "detect_leaks=0"; }
#endif
