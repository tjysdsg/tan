#ifndef __TAN_SRC_ANALYSIS_H__
#define __TAN_SRC_ANALYSIS_H__
#include "base.h"
#include "src/ast/ast_node.h"

namespace tanlang {

/// \section General

str get_name(ASTNodePtr p);
size_t get_n_children(ASTNodePtr p);
bool is_lvalue(ASTNodePtr p);
bool is_lvalue(ASTTyPtr p);

/// \section AST factories

/// \subsection Declarations

ASTNodePtr ast_create_arg_decl();
ASTNodePtr ast_create_var_decl();
ASTNodePtr ast_create_arg_decl(const str &name, ASTTyPtr ty);
ASTNodePtr ast_create_var_decl(const str &name, ASTTyPtr ty);

/// \subsection Literals

ASTNodePtr ast_create_string_literal();
ASTNodePtr ast_create_string_literal(const str &);
ASTNodePtr ast_create_array_literal();
ASTNodePtr ast_create_numeric_literal();

/// \subsection Binary ops

ASTNodePtr ast_create_arithmetic(const str &op);
ASTNodePtr ast_create_comparison(const str &op);

/// \subsection Others

ASTNodePtr ast_create_program();
ASTNodePtr ast_create_statement();
ASTNodePtr ast_create_identifier();
ASTNodePtr ast_create_identifier(const str &name);
ASTTyPtr ast_create_ty();

/// \section Literals

ASTNodePtr create_numeric_literal(uint64_t val, bool is_unsigned = false);
ASTNodePtr create_numeric_literal(double val);

/// \section Types

ASTTyPtr create_ty(Ty t, vector<ASTNodePtr> sub_tys = {}, bool is_lvalue = false);
ASTTyPtr get_ty(ASTNodePtr p);
str get_type_name(ASTNodePtr p);
ASTTyPtr get_contained_ty(ASTTyPtr p);
ASTTyPtr get_ptr_to(ASTTyPtr p);
size_t get_size_bits(ASTTyPtr p);
bool is_ptr(ASTTyPtr p);
bool is_float(ASTTyPtr p);
bool is_floating(ASTTyPtr p);
bool is_double(ASTTyPtr p);
bool is_int(ASTTyPtr p);
bool is_bool(ASTTyPtr p);
bool is_enum(ASTTyPtr p);
bool is_unsigned(ASTTyPtr p);
bool is_struct(ASTTyPtr p);
bool is_array(ASTTyPtr p);
size_t get_struct_member_index(ASTTyPtr p, str name);
ASTTyPtr get_struct_member_ty(ASTTyPtr p, size_t i);
void resolve_ty(CompilerSession *cs, ASTTyPtr p);

/// \section Analysis

void analyze(CompilerSession *cs, ASTNodePtr p);

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_H__
