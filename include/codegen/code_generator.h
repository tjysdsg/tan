#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#include "base.h"
#include "ast/fwd.h"
#include "llvm_api/llvm_include.h"

namespace tanlang {

class ASTBase;

class CodeGenerator {
public:
  CodeGenerator() = delete;
  CodeGenerator(SourceManager *sm, llvm::TargetMachine *target_machine);
  ~CodeGenerator();
  void emit_to_file(const str &filename);
  void dump_ir() const;
  llvm::Value *codegen(ASTBase *p);

  /**
   * \brief Convert a value to from orig type to dest type.
   * \details Returns nullptr if failed to convert.
   * \param dest Destination type.
   * \param expr Original expression.
   * \return Converted value if convertible, otherwise `nullptr`. Note that the returned value is always rvalue. To
   * get an lvalue, create a temporary variable and store the value to it.
   */
  llvm::Value *convert_llvm_type_to(Expr *expr, Type *dest);

  /// Create a load instruction if the type is lvalue. Otherwise return the original value.
  llvm::Value *load_if_is_lvalue(Expr *expr);

  llvm::Type *to_llvm_type(Type *p);
  llvm::Metadata *to_llvm_metadata(Type *p, SrcLoc loc);
  llvm::DISubroutineType *create_function_debug_info_type(llvm::Metadata *ret, vector<llvm::Metadata *> args);

private:
  SourceManager *_sm = nullptr;

  umap<Type *, llvm::Type *> _llvm_type_cache{};
  umap<ASTBase *, llvm::Value *> _llvm_value_cache{};

  /// LLVM things
  llvm::IRBuilder<> *_builder = nullptr;
  llvm::DIBuilder *_di_builder = nullptr;
  llvm::LLVMContext *_llvm_ctx = nullptr;
  llvm::Module *_module = nullptr;
  vector<llvm::DIScope *> _di_scope{};
  llvm::TargetMachine *_target_machine = nullptr;
  llvm::DICompileUnit *_di_cu = nullptr;
  llvm::DIFile *_di_file = nullptr;

private:
  [[noreturn]] void error(ASTBase *p, const str &message);
  llvm::DIScope *get_current_di_scope() const;
  void push_di_scope(llvm::DIScope *scope);
  void pop_di_scope();
  void set_current_debug_location(ASTBase *p);
  llvm::DebugLoc debug_loc_of_node(ASTBase *p, llvm::MDNode *scope = nullptr);

  /**
   * \brief create_ty an `alloca` instruction in the beginning of a block.
   * \param block BasicBlock to insert to.
   * \param type Intended type to store.
   * \param name Name of the `alloca` instruction.
   * \param size size of the array if greater than 1
   */
  AllocaInst *create_block_alloca(BasicBlock *block, llvm::Type *type, size_t size = 1, const str &name = "");

  llvm::Value *codegen_func_call(ASTBase *_p);
  llvm::Value *codegen_func_prototype(FunctionDecl *p, bool import = false);
  llvm::Value *codegen_func_decl(FunctionDecl *p);
  llvm::Value *codegen_bnot(ASTBase *_p);
  llvm::Value *codegen_lnot(ASTBase *_p);
  llvm::Value *codegen_return(ASTBase *_p);
  llvm::Value *codegen_var_arg_decl(ASTBase *_p);
  llvm::Value *codegen_address_of(ASTBase *_p);
  llvm::Value *codegen_parenthesis(ASTBase *_p);
  llvm::Value *codegen_import(ASTBase *_p);
  llvm::Value *codegen_intrinsic(Intrinsic *p);
  llvm::Value *codegen_constructor(Constructor *p);
  llvm::Value *codegen_type_instantiation(Type *p);
  llvm::Value *codegen_literals(ASTBase *_p);
  llvm::Value *codegen_stmt(ASTBase *_p);
  llvm::Value *codegen_uop(ASTBase *_p);
  llvm::Value *codegen_bop(ASTBase *_p);
  llvm::Value *codegen_assignment(ASTBase *_p);
  llvm::Value *codegen_arithmetic(ASTBase *_p);
  llvm::Value *codegen_comparison(ASTBase *_p);
  llvm::Value *codegen_relop(ASTBase *_p);
  llvm::Value *codegen_cast(ASTBase *_p);
  llvm::Value *codegen_var_ref(ASTBase *_p);
  llvm::Value *codegen_identifier(ASTBase *_p);
  llvm::Value *codegen_binary_or_unary(ASTBase *_p);
  llvm::Value *codegen_break_continue(ASTBase *_p);
  llvm::Value *codegen_loop(ASTBase *_p);
  llvm::Value *codegen_if(ASTBase *_p);
  llvm::Value *codegen_member_access(MemberAccess *p);
  llvm::Value *codegen_ptr_deref(UnaryOperator *p);
};

} // namespace tanlang

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_H__
