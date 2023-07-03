#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#include "base.h"
#include "ast/fwd.h"
#include "llvm_api/llvm_include.h"
#include "common/compilation_unit.h"
#include "common/compiler_action.h"

namespace tanlang {

class ASTBase;
class StructType;

class CodeGenerator final : public CompilerAction<CodeGenerator, CompilationUnit *, llvm::Value *> {
public:
  CodeGenerator() = delete;
  explicit CodeGenerator(TargetMachine *target_machine);
  ~CodeGenerator();

  void init(CompilationUnit *cu) override;
  llvm::Value *run_impl(CompilationUnit *cu);
  llvm::Value *cached_visit(ASTBase *p);
  void default_visit(ASTBase *) override;

  void emit_to_file(const str &filename);
  void run_passes();
  void dump_ir() const;

private:
  [[noreturn]] void error(ErrorType type, ASTBase *p, const str &message);

private:
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
  llvm::Metadata *to_llvm_metadata(Type *p, uint32_t loc);
  llvm::DISubroutineType *create_function_debug_info_type(llvm::Metadata *ret, vector<llvm::Metadata *> args);

private:
  CompilationUnit *_cu = nullptr;
  SourceManager *_sm = nullptr;

  umap<Type *, llvm::Type *> _llvm_type_cache{};
  umap<Type *, llvm::Metadata *> _llvm_meta_cache{};
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

private:
  Value *codegen_var_arg_decl(Decl *p);
  Value *codegen_struct_default_value(StructType *ty);
  Value *codegen_type_default_value(Type *p);
  Value *codegen_literals(Literal *p);
  Value *codegen_func_prototype(FunctionDecl *p, bool import_ = false);
  Value *codegen_ptr_deref(UnaryOperator *p);
  Value *codegen_relop(BinaryOperator *p);
  Value *codegen_bnot(UnaryOperator *p);
  Value *codegen_lnot(UnaryOperator *p);
  Value *codegen_address_of(UnaryOperator *p);
  Value *codegen_arithmetic(BinaryOperator *p);
  Value *codegen_comparison(BinaryOperator *p);
  Value *codegen_member_access(BinaryOperator *p);

public:
  DECLARE_AST_VISITOR_IMPL(Program);
  DECLARE_AST_VISITOR_IMPL(Identifier);
  DECLARE_AST_VISITOR_IMPL(Parenthesis);
  DECLARE_AST_VISITOR_IMPL(If);
  DECLARE_AST_VISITOR_IMPL(VarDecl);
  DECLARE_AST_VISITOR_IMPL(ArgDecl);
  DECLARE_AST_VISITOR_IMPL(Return);
  DECLARE_AST_VISITOR_IMPL(CompoundStmt);
  DECLARE_AST_VISITOR_IMPL(BinaryOrUnary);
  DECLARE_AST_VISITOR_IMPL(BinaryOperator);
  DECLARE_AST_VISITOR_IMPL(UnaryOperator);
  DECLARE_AST_VISITOR_IMPL(Cast);
  DECLARE_AST_VISITOR_IMPL(Assignment);
  DECLARE_AST_VISITOR_IMPL(FunctionCall);
  DECLARE_AST_VISITOR_IMPL(FunctionDecl);
  DECLARE_AST_VISITOR_IMPL(Import);
  DECLARE_AST_VISITOR_IMPL(Intrinsic);
  DECLARE_AST_VISITOR_IMPL(ArrayLiteral);
  DECLARE_AST_VISITOR_IMPL(CharLiteral);
  DECLARE_AST_VISITOR_IMPL(BoolLiteral);
  DECLARE_AST_VISITOR_IMPL(IntegerLiteral);
  DECLARE_AST_VISITOR_IMPL(FloatLiteral);
  DECLARE_AST_VISITOR_IMPL(StringLiteral);
  DECLARE_AST_VISITOR_IMPL(NullPointerLiteral);
  // DECLARE_AST_VISITOR_IMPL(MemberAccess);
  DECLARE_AST_VISITOR_IMPL(StructDecl);
  DECLARE_AST_VISITOR_IMPL(Loop);
  DECLARE_AST_VISITOR_IMPL(BreakContinue);
  DECLARE_AST_VISITOR_IMPL(VarRef);
};

} // namespace tanlang

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_H__
