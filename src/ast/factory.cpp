#include "src/ast/factory.h"
#include "src/common.h"
#include "intrinsic.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

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

} // namespace tanlang
