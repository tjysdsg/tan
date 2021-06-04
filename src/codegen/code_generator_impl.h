#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#include "compiler_session.h"
#include "base.h"
#include "src/analysis/ast_helper.h"
#include "src/ast/fwd.h"

namespace llvm {
class Value;
}

namespace tanlang {

using llvm::Value;

class CodeGeneratorImpl {
public:
  CodeGeneratorImpl() = delete;
  explicit CodeGeneratorImpl(CompilerSession *cs);
  Value *codegen(ASTBase *p);

private:
  void set_current_debug_location(ASTBase *p);

  Value *codegen_stmt(ASTBase *p);
  Value *codegen_bop(ASTBase *p);
  Value *codegen_uop(ASTBase *p);
  Value *codegen_arithmetic(ASTBase *p);
  Value *codegen_lnot(ASTBase *p);
  Value *codegen_bnot(ASTBase *p);
  Value *codegen_return(ASTBase *p);
  Value *codegen_comparison(ASTBase *p);
  Value *codegen_assignment(ASTBase *p);
  Value *codegen_cast(ASTBase *p);
  Value *codegen_var_arg_decl(ASTBase *p);
  Value *codegen_address_of(ASTBase *p);
  Value *codegen_parenthesis(ASTBase *p);
  Value *codegen_break_continue(ASTBase *p);
  Value *codegen_loop(ASTBase *p);
  Value *codegen_if(ASTBase *p);
  Value *codegen_func_call(ASTBase *p);
  Value *codegen_import(ASTBase *p);
  Value *codegen_literals(ASTBase *p);
  Value *codegen_identifier(ASTBase *p);

  Value *codegen_ty(ASTType *p);
  Value *codegen_func_prototype(FunctionDecl *p, bool import = false);
  Value *codegen_func_decl(FunctionDecl *p);
  Value *codegen_intrinsic(Intrinsic *p);
  Value *codegen_member_access(MemberAccess *p);

  [[noreturn]] void report_error(ASTBase *p, const str &message);

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
