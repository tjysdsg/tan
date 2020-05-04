#include "src/common.h"
#include "src/llvm_include.h"

namespace tanlang {

AllocaInst *create_block_alloca(BasicBlock *block, Type *type, size_t size, const std::string &name) {
  block = &block->getParent()->getEntryBlock();
  IRBuilder<> tmp_builder(block, block->begin());
  if (size <= 1) {
    return tmp_builder.CreateAlloca(type, nullptr, name);
  } else {
    return tmp_builder.CreateAlloca(type, tmp_builder.getInt32((unsigned) size), name);
  };
}

bool is_ast_type_in(ASTType t, std::initializer_list<ASTType> list) {
  return std::any_of(list.begin(), list.end(), [t](ASTType i) { return i == t; });
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

