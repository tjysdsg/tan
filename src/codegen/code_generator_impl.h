#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#include "base.h"

namespace llvm {
class Value;
}

namespace tanlang {

class CompilerSession;
AST_FWD_DECL(ParsableASTNode);
AST_FWD_DECL(ASTNode);
AST_FWD_DECL(ASTTy);
AST_FWD_DECL(ASTFunction);
AST_FWD_DECL(ASTMemberAccess);
AST_FWD_DECL(Intrinsic);

using llvm::Value;

class CodeGeneratorImpl {
public:
  CodeGeneratorImpl() = delete;
  explicit CodeGeneratorImpl(CompilerSession *cs);
  Value *codegen(const ASTNodePtr &p);

private:
  void set_current_debug_location(ASTNodePtr p);
  Value *codegen_arithmetic(ASTNodePtr p);
  Value *codegen_lnot(ASTNodePtr p);
  Value *codegen_bnot(ASTNodePtr p);
  Value *codegen_return(ASTNodePtr p);
  Value *codegen_comparison(ASTNodePtr p);
  Value *codegen_assignment(ASTNodePtr p);
  Value *codegen_cast(ASTNodePtr p);
  Value *codegen_ty(ASTTyPtr p);
  Value *codegen_var_arg_decl(ASTNodePtr p);
  Value *codegen_address_of(ASTNodePtr p);
  Value *codegen_parenthesis(ASTNodePtr p);
  Value *codegen_break_continue(ASTNodePtr p);
  Value *codegen_loop(ASTNodePtr p);
  Value *codegen_if(ASTNodePtr p);
  Value *codegen_func_call(ASTNodePtr p);
  Value *codegen_func_prototype(ASTFunctionPtr p, bool import = false);
  Value *codegen_func_decl(ASTFunctionPtr p);
  Value *codegen_import(ASTNodePtr p);
  Value *codegen_literals(ASTNodePtr p);
  Value *codegen_intrinsic(IntrinsicPtr p);
  Value *codegen_member_access(ASTMemberAccessPtr p);

private:
  CompilerSession *_cs = nullptr;
};

}

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
