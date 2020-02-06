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

Type *typename_to_llvm_type(const std::string &type_name, CompilerSession *parser_context) {
  if (type_name == "int" || type_name == "i32" || type_name == "u32") {
    return parser_context->_builder->getInt32Ty();
  } else if (type_name == "i64" || type_name == "u64") {
    return parser_context->_builder->getInt64Ty();
  } else if (type_name == "i16" || type_name == "u16") {
    return parser_context->_builder->getInt16Ty();
  } else if (type_name == "float") {
    return parser_context->_builder->getFloatTy();
  } else if (type_name == "double") {
    return parser_context->_builder->getDoubleTy();
  } else {
    return nullptr;
  }
}

}
