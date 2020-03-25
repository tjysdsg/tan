#include "src/ast/ast_array.h"
#include "src/ast/ast_nptr.h"
#include "src/ast/common.h"

namespace tanlang {

Value *ASTArrayLiteral::codegen(CompilerSession *compiler_session) {
  Ty ty = ast_cast<ASTLiteral>(_children[0])->get_ty();
  auto nptr = std::make_shared<ASTNPtr>(ty, _children.size());
  nptr->codegen(compiler_session); // register type
  Type *nptr_type = nptr->to_llvm_type(compiler_session);
  auto ret = create_block_alloca(compiler_session->get_builder()->GetInsertBlock(), nptr_type, "array_literal");
  // TODO: set initial values
  return ret;
}

llvm::Value *ASTArrayLiteral::get_llvm_value(CompilerSession *) const {
  return _llvm_value;
}

std::string ASTArrayLiteral::get_type_name() const {
  // TODO: add element type name
  return "array";
}

llvm::Type *ASTArrayLiteral::to_llvm_type(CompilerSession *) const {
  return _llvm_type;
}

} // namespace tanlang
