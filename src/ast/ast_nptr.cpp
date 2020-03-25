#include "src/ast/ast_expr.h"
#include "src/ast/ast_nptr.h"
#include "src/ast/factory.h"

namespace tanlang {

ASTNPtr::ASTNPtr(Ty orig_type, int n) : ASTStruct(nullptr) {
  auto orig_ty = std::make_shared<ASTTy>(nullptr);
  orig_ty->_ty = orig_type;
  auto ptr_ty = std::make_shared<ASTTy>(nullptr);
  ptr_ty->_children.push_back(orig_ty);
  ptr_ty->_ty = Ty::POINTER;
  auto member_ptr = ASTFactory<ASTVarDecl>::Create("ptr", ptr_ty);
  _children.push_back(ptr_ty); // first element of the struct

  _children.push_back(std::make_shared<ASTNumberLiteral>(n));
}

} // namespace tanlang
