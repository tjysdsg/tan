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

bool is_llvm_type_same(llvm::Type *t1, llvm::Type *t2) {
  if (t1->getTypeID() != t2->getTypeID()) { return false; }
  auto n = t1->getNumContainedTypes();
  if (n > 0) {
    if (n != t2->getNumContainedTypes()) { return false; }
    for (unsigned i = 0; i < n; ++i) {
      if (!is_llvm_type_same(t1->getContainedType(i), t2->getContainedType(i))) { return false; }
    }
  } else {
    return t1->getPrimitiveSizeInBits() == t2->getPrimitiveSizeInBits();
  }
  return true;
}

} // namespace tanlang

