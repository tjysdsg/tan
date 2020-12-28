#ifndef __TAN_SRC_ANALYSIS_H__
#define __TAN_SRC_ANALYSIS_H__
#include "base.h"
#include "src/ast/ast_node.h"

namespace tanlang {

/// \section General

size_t get_n_children(ASTNodePtr p);
bool is_lvalue(ASTNodePtr p);
ASTNodePtr get_id_referred(CompilerSession *cs, ASTNodePtr p);

/// \section AST factories

/// \subsection Declarations

ASTNodePtr ast_create_arg_decl(CompilerSession *cs);
ASTNodePtr ast_create_var_decl(CompilerSession *cs);
ASTNodePtr ast_create_arg_decl(CompilerSession *cs, const str &name, ASTTyPtr ty);
ASTNodePtr ast_create_var_decl(CompilerSession *cs, const str &name, ASTTyPtr ty);
ASTNodePtr ast_create_func_decl(CompilerSession *cs);

/// \subsection Literals

ASTNodePtr ast_create_string_literal(CompilerSession *cs);
ASTNodePtr ast_create_string_literal(CompilerSession *cs, const str &);
ASTNodePtr ast_create_array_literal(CompilerSession *cs);
ASTNodePtr ast_create_numeric_literal(CompilerSession *cs);
ASTNodePtr ast_create_cast(CompilerSession *cs);

/// \subsection Binary ops

ASTNodePtr ast_create_arithmetic(CompilerSession *cs, const str &op);
ASTNodePtr ast_create_comparison(CompilerSession *cs, const str &op);
ASTNodePtr ast_create_assignment(CompilerSession *cs);
ASTNodePtr ast_create_member_access(CompilerSession *cs);

/// \subsection Unary ops

ASTNodePtr ast_create_return(CompilerSession *cs);

/// \subsection Ambiguous ops
/// The type of these operators/expression is undetermined before parsing

ASTNodePtr ast_create_ampersand(CompilerSession *cs);
ASTNodePtr ast_create_not(CompilerSession *cs);

/// \subsection Keywords
ASTNodePtr ast_create_if(CompilerSession *cs);

/// \subsection Others

ASTNodePtr ast_create_program(CompilerSession *cs);
ASTNodePtr ast_create_statement(CompilerSession *cs);
ASTNodePtr ast_create_identifier(CompilerSession *cs);
ASTNodePtr ast_create_identifier(CompilerSession *cs, const str &name);
ASTNodePtr ast_create_parenthesis(CompilerSession *cs);
ASTTyPtr ast_create_ty(CompilerSession *cs);

/// \section Literals

ASTNodePtr ast_create_numeric_literal(CompilerSession *cs, uint64_t val, bool is_unsigned = false);
ASTNodePtr ast_create_numeric_literal(CompilerSession *cs, double val);
ASTNodePtr ast_create_char_literal(CompilerSession *cs);
ASTNodePtr ast_create_char_literal(CompilerSession *cs, char c);

/// \section Types

llvm::Type *to_llvm_type(CompilerSession *cs, ASTTyPtr p);
llvm::Metadata *to_llvm_meta(CompilerSession *cs, ASTTyPtr p);
str get_type_name(ASTNodePtr p);
ASTTyPtr create_ty(CompilerSession *cs, Ty t, vector<ASTNodePtr> sub_tys = {}, bool is_lvalue = false);
ASTTyPtr get_contained_ty(CompilerSession *cs, ASTTyPtr p);
ASTTyPtr get_ptr_to(CompilerSession *cs, ASTTyPtr p);
size_t get_size_bits(CompilerSession *cs, ASTTyPtr p);
size_t get_struct_member_index(CompilerSession *cs, ASTTyPtr p, str name);
ASTTyPtr get_struct_member_ty(CompilerSession *cs, ASTTyPtr p, size_t i);
void resolve_ty(CompilerSession *cs, ASTTyPtr p);

/// \section Analysis

void analyze(CompilerSession *cs, const ASTNodePtr &p);

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_H__
