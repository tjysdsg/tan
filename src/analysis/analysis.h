#ifndef __TAN_SRC_ANALYSIS_H__
#define __TAN_SRC_ANALYSIS_H__
#include "base.h"
#include "src/ast/ast_node.h"

namespace tanlang {

size_t get_n_children(const ASTNodePtr &p);
bool is_lvalue(const ASTNodePtr &p);
ASTNodePtr get_id_referred(CompilerSession *cs, const ASTNodePtr &p);

llvm::Type *to_llvm_type(CompilerSession *cs, const ASTTyPtr &p);
llvm::Metadata *to_llvm_meta(CompilerSession *cs, const ASTTyPtr &p);
str get_type_name(const ASTNodePtr &p);
ASTTyPtr get_contained_ty(CompilerSession *cs, const ASTTyPtr &p);
ASTTyPtr get_ptr_to(CompilerSession *cs, const ASTTyPtr &p);
size_t get_size_bits(CompilerSession *cs, ASTTyPtr p);
size_t get_struct_member_index(const ASTTyPtr &p, const str &name);
ASTTyPtr get_struct_member_ty(const ASTTyPtr &p, size_t i);
void resolve_ty(CompilerSession *cs, const ASTTyPtr &p);

void analyze(CompilerSession *cs, const ASTNodePtr &p);

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_H__
