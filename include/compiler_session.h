#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "base.h"

namespace tanlang {

struct Scope;
AST_FWD_DECL(ASTType);
AST_FWD_DECL(ASTFunction);
AST_FWD_DECL(FunctionTable);
AST_FWD_DECL(ASTLoop);

/**
 * \class CompilerSession
 * \brief Wraps all LLVM classes used for code generation
 * */
class CompilerSession final {
public:
  static void AddPublicFunction(const str &filename, ASTFunctionPtr func);
  static vector<ASTFunctionPtr> GetPublicFunctions(const str &filename);

private:
  /**
   * \brief Function table for each source files
   * \details filename -> (function name -> FunctionTable)
   * */
  static inline umap<str, FunctionTablePtr> public_func{};

public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;
  CompilerSession(const str &module_name, TargetMachine *target_machine);
  ~CompilerSession();

public:
  /**
   * \brief Get current scope
   * \see Scope
   * */
  ptr<Scope> get_current_scope();

  /**
   * \brief create_ty a new scope
   * \see Scope
   * */
  ptr<Scope> push_scope();

  /**
   * \brief Set the current scope
   * \see Scope
   * */
  void push_scope(ptr<Scope>);

  /**
   * \brief Pop the current scope
   * \see Scope
   * */
  ptr<Scope> pop_scope();
  [[nodiscard]] DIScope *get_current_di_scope() const;
  void push_di_scope(DIScope *scope);
  void pop_di_scope();

  /**
   * \brief Add a named ASTNode so that others can loop it up using CompilerSession::get
   * */
  void add(const str &name, ptr<ASTNode> value);

  /**
   * \brief Register a variable
   * */
  void set(const str &name, ptr<ASTNode> value);

  /**
   * \brief look up the variable table in the current and parent scopes
   * \details This function starts by searching the current scope. If the target is not found in current scope,
   * search the parent scope, repeat the process until found. Return nullptr if not found in all visible scopes.
   * */
  ASTNodePtr get(const str &name);

  /**
   * \brief Register a type
   * */
  void set_type(const str &name, ASTTypePtr value);

  /**
   * \brief Look up type table
   * \param name typename
   */
  ASTTypePtr get_type(const str &name);

  LLVMContext *get_context();
  Module *get_module();
  void emit_object(const str &filename);

  /**
   * \brief Get the size of a pointer on the current machine
   * \details This is equivalent as llvm::TargetMachine->PointerSizeInBits()
   * */
  [[nodiscard]] unsigned get_ptr_size() const;

  /**
   * \brief Add a function AST to the current file's function table
   * \details This will not add anything to the public function table, to do that,
   * call CompilerSession::AddPublicFunction
   * */
  void add_function(ASTFunctionPtr func);
  vector<ASTFunctionPtr> get_functions(const str &name);
  [[nodiscard]] ptr<ASTLoop> get_current_loop() const;
  void set_current_loop(ptr<ASTLoop>);
  [[nodiscard]] DIFile *get_di_file() const;
  [[nodiscard]] DICompileUnit *get_di_cu() const;
  void set_current_debug_location(size_t l, size_t c);

public:
  str _filename = "";
  IRBuilder<> *_builder = nullptr; /// IR builder
  DIBuilder *_di_builder = nullptr; /// Debug information builder
  Token *_current_token = nullptr; /// Used for error messages

private:
  umap<str, ASTTypePtr> _type_table{};

  LLVMContext *_context = nullptr;
  Module *_module = nullptr;
  vector<ptr<Scope>> _scope{}; // TODO: use tree for scope
  vector<DIScope *> _di_scope{};
  std::unique_ptr<FunctionPassManager> _fpm{};
  std::unique_ptr<PassManager> _mpm{};
  TargetMachine *_target_machine = nullptr;
  DICompileUnit *_di_cu = nullptr;
  DIFile *_di_file = nullptr;
  FunctionTablePtr _function_table = nullptr;

  /**
   * The control flow in current scope, used by break and continue
   * */
  ptr<ASTLoop> _current_loop = nullptr;

private:
  void initialize_scope();
  void init_llvm();
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
