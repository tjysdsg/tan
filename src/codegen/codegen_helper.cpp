#include "src/codegen/codegen_helper.h"

using namespace tanlang;

Constant *CodegenHelper::CodegenIntegerLiteral(CompilerSession *cs,
    uint64_t val,
    unsigned int bit_size,
    bool is_unsigned) {
  return ConstantInt::get(cs->_builder->getIntNTy(bit_size), val, !is_unsigned);
}
