#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
#include "compiler_session.h"
#include "base.h"
#include "src/analysis/ast_helper.h"

namespace llvm {
class Value;
}

namespace tanlang {

class CompilerSession;
AST_FWD_DECL(ASTBase);
AST_FWD_DECL(ASTType);
AST_FWD_DECL(FunctionDecl);
AST_FWD_DECL(Intrinsic);
AST_FWD_DECL(MemberAccess);

using llvm::Value;

class CodeGeneratorImpl {
public:
  CodeGeneratorImpl() = delete;
  explicit CodeGeneratorImpl(CompilerSession *cs);
  Value *codegen(const ASTBasePtr &p);

private:
  void set_current_debug_location(ASTBasePtr p);

  Value *codegen_stmt(const ASTBasePtr &p);
  Value *codegen_bop(const ASTBasePtr &p);
  Value *codegen_uop(const ASTBasePtr &p);
  Value *codegen_arithmetic(const ASTBasePtr &p);
  Value *codegen_lnot(const ASTBasePtr &p);
  Value *codegen_bnot(const ASTBasePtr &p);
  Value *codegen_return(const ASTBasePtr &p);
  Value *codegen_comparison(const ASTBasePtr &p);
  Value *codegen_assignment(const ASTBasePtr &p);
  Value *codegen_cast(const ASTBasePtr &p);
  Value *codegen_var_arg_decl(const ASTBasePtr &p);
  Value *codegen_address_of(const ASTBasePtr &p);
  Value *codegen_parenthesis(const ASTBasePtr &p);
  Value *codegen_break_continue(const ASTBasePtr &p);
  Value *codegen_loop(const ASTBasePtr &p);
  Value *codegen_if(const ASTBasePtr &p);
  Value *codegen_func_call(const ASTBasePtr &p);
  Value *codegen_import(const ASTBasePtr &p);
  Value *codegen_literals(const ASTBasePtr &p);
  Value *codegen_identifier(const ASTBasePtr &p);

  Value *codegen_ty(const ASTTypePtr &p);
  Value *codegen_func_prototype(const FunctionDeclPtr &p, bool import = false);
  Value *codegen_func_decl(const FunctionDeclPtr &p);
  Value *codegen_intrinsic(const IntrinsicPtr &p);
  Value *codegen_member_access(const MemberAccessPtr &p);

  [[noreturn]] void report_error(const ASTBasePtr &p, const str &message);

private:
  CompilerSession *_cs = nullptr;
  ASTHelper _h;
};

}

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_IMPL_H__
