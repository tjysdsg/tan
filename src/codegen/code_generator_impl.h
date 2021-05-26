#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#include "src/analysis/ast_helper.h"
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
  Value *codegen_arithmetic(const ASTNodePtr &p);
  Value *codegen_lnot(const ASTNodePtr &p);
  Value *codegen_bnot(const ASTNodePtr &p);
  Value *codegen_return(const ASTNodePtr &p);
  Value *codegen_comparison(const ASTNodePtr &p);
  Value *codegen_assignment(const ASTNodePtr &p);
  Value *codegen_cast(const ASTNodePtr &p);
  Value *codegen_ty(const ASTTyPtr &p);
  Value *codegen_var_arg_decl(const ASTNodePtr &p);
  Value *codegen_address_of(const ASTNodePtr &p);
  Value *codegen_parenthesis(const ASTNodePtr &p);
  Value *codegen_break_continue(const ASTNodePtr &p);
  Value *codegen_loop(const ASTNodePtr &p);
  Value *codegen_if(const ASTNodePtr &p);
  Value *codegen_func_call(const ASTNodePtr &p);
  Value *codegen_func_prototype(const ASTFunctionPtr &p, bool import = false);
  Value *codegen_func_decl(const ASTFunctionPtr &p);
  Value *codegen_import(const ASTNodePtr &p);
  Value *codegen_literals(const ASTNodePtr &p);
  Value *codegen_intrinsic(const IntrinsicPtr &p);
  Value *codegen_member_access(const ASTMemberAccessPtr &p);
  [[noreturn]] void report_error(const ParsableASTNodePtr &p, const str &message);

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
