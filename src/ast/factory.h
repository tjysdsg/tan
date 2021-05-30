#ifndef __TAN_SRC_AST_FACTORY_H__
#define __TAN_SRC_AST_FACTORY_H__

#include "base.h"

namespace tanlang {

/// \section Ops

ASTNodePtr ast_create_cast(CompilerSession *cs);
ASTNodePtr ast_create_arithmetic(CompilerSession *cs, const str &op);
ASTNodePtr ast_create_comparison(CompilerSession *cs, const str &op);
ASTNodePtr ast_create_assignment(CompilerSession *cs);
ASTNodePtr ast_create_member_access(CompilerSession *cs);
ASTNodePtr ast_create_return(CompilerSession *cs);
ASTNodePtr ast_create_address_of(CompilerSession *cs, ASTBasePtr p);

/// \section Ambiguous ops
/// The type of these operators/expression is undetermined before parsing

ASTNodePtr ast_create_ampersand(CompilerSession *cs);
ASTNodePtr ast_create_not(CompilerSession *cs);

/// \section Control flow
ASTNodePtr ast_create_if(CompilerSession *cs);
ASTNodePtr ast_create_else(CompilerSession *cs);
ASTNodePtr ast_create_loop(CompilerSession *cs);
ASTNodePtr ast_create_break(CompilerSession *cs);
ASTNodePtr ast_create_continue(CompilerSession *cs);

/// \section Others

ASTNodePtr ast_create_import(CompilerSession *cs);
ASTNodePtr ast_create_intrinsic(CompilerSession *cs);
ASTNodePtr ast_create_identifier(CompilerSession *cs);
ASTNodePtr ast_create_identifier(CompilerSession *cs, const str &name);
ASTNodePtr ast_create_parenthesis(CompilerSession *cs);
ASTNodePtr ast_create_func_call(CompilerSession *cs);

} // namespace tanlang

#endif //__TAN_SRC_AST_FACTORY_H__
