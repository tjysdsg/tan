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
  // TODO: type name
  _children.push_back(ASTFactory<ASTIdentifier>::Create("nptr"));
  _children.push_back(member_ptr); // first element of the struct

  auto size = std::make_shared<ASTNumberLiteral>(n);
  auto size_ty = std::make_shared<ASTTy>(nullptr);
  size_ty->_ty = TY_OR(Ty::INT, Ty::BIT32);
  auto member_size = ASTFactory<ASTVarDecl>::Create("size", size_ty, ASTFactory<ASTNumberLiteral>::Create(n));
  _children.push_back(member_size);
}

} // namespace tanlang
