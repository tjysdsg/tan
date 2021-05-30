#ifndef __TAN_SRC_AST_FACTORY_H__
#define __TAN_SRC_AST_FACTORY_H__

#include "base.h"

namespace tanlang {

/// \section Ops

ASTNodePtr ast_create_cast(CompilerSession *cs);
ASTNodePtr ast_create_member_access(CompilerSession *cs);

/// \section Control flow
ASTNodePtr ast_create_if(CompilerSession *cs);
ASTNodePtr ast_create_else(CompilerSession *cs);
ASTNodePtr ast_create_loop(CompilerSession *cs);
ASTNodePtr ast_create_break(CompilerSession *cs);
ASTNodePtr ast_create_continue(CompilerSession *cs);

/// \section Others

ASTNodePtr ast_create_import(CompilerSession *cs);
ASTNodePtr ast_create_intrinsic(CompilerSession *cs);
ASTNodePtr ast_create_func_call(CompilerSession *cs);

} // namespace tanlang

#endif //__TAN_SRC_AST_FACTORY_H__
