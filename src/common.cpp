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

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list) {
  return std::any_of(list.begin(), list.end(), [t](ASTType i) { return i == t; });
}

bool is_string_in(std::string_view s, std::initializer_list<std::string_view> list) {
  return std::any_of(list.begin(), list.end(), [s](std::string_view i) { return i == s; });
}

bool is_string_in(std::string_view s, const vector<str> &list) {
  return std::any_of(list.begin(), list.end(), [s](std::string_view i) { return i == s; });
}

} // namespace tanlang

