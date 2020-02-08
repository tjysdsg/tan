#include "src/ast/common.h"

namespace tanlang {

AllocaInst *create_block_alloca(BasicBlock *block, Type *type, const std::string &name) {
  IRBuilder<> tmp_builder(block, block->begin());
  return tmp_builder.CreateAlloca(type, nullptr, name);
}

struct Equal {
  const ASTType val;
  Equal() = delete;
  explicit Equal(ASTType v) : val(v) {}
  bool operator()(ASTType v) const { return v == val; }
};

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list) {
  return std::any_of(list.begin(), list.end(), Equal(t));
}

} // namespace tanlang

