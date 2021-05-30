#include "src/ast/factory.h"
#include "src/common.h"
#include "src/ast/ast_member_access.h"
#include "intrinsic.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

/// \section Ops

ASTNodePtr ast_create_arithmetic(CompilerSession *, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::INVALID, 0);
  ret->set_lbp(ASTNode::OpPrecedence[ret->get_node_type()]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_cast(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::CAST, ASTNode::OpPrecedence[ASTNodeType::CAST]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_member_access(CompilerSession *) {
  auto ret = make_ptr<ASTMemberAccess>(ASTNodeType::MEMBER_ACCESS, ASTNode::OpPrecedence[ASTNodeType::MEMBER_ACCESS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

/// \section Control flow

ASTNodePtr ast_create_if(CompilerSession *) {
  auto ret = make_ptr<ASTIf>(ASTNodeType::IF, ASTNode::OpPrecedence[ASTNodeType::IF]);
  return ret;
}

ASTNodePtr ast_create_else(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::ELSE, ASTNode::OpPrecedence[ASTNodeType::ELSE]);
  return ret;
}

ASTNodePtr ast_create_loop(CompilerSession *) {
  auto ret = make_ptr<ASTLoop>();
  return ret;
}

ASTNodePtr ast_create_break(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::BREAK, 0);
  return ret;
}

ASTNodePtr ast_create_continue(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::CONTINUE, 0);
  return ret;
}

/// \section Others

ASTNodePtr ast_create_import(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::IMPORT, 0);
  ret->_is_named = true; /// name is the file imported
  return ret;
}

ASTNodePtr ast_create_intrinsic(CompilerSession *) {
  auto ret = make_ptr<Intrinsic>();
  return ret;
}

ASTNodePtr ast_create_func_call(CompilerSession *) {
  return make_ptr<FunctionCall>();
}

} // namespace tanlang
