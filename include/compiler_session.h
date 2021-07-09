#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "base.h"
#include "src/ast/fwd.h"

namespace tanlang {

/**
 * \class CompilerSession
 * \brief Wraps all LLVM classes used for code generation
 * */
class CompilerSession final {
public:
  static void AddPublicFunction(const str &filename, FunctionDecl *func);
  static vector<FunctionDecl *> GetPublicFunctions(const str &filename);

private:
  /**
   * \brief Function table for each source files
   * \details filename -> (function name -> FunctionTable)
   * */
  static inline umap<str, FunctionTable *> public_func{};

public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;
  CompilerSession(const str &module_name, TargetMachine *target_machine);
  ~CompilerSession();

public:
  SourceManager *get_source_manager() const;
  void set_source_manager(SourceManager *source_manager);
  str get_source_location_str(SourceTraceable *p) const;

  /**
   * \brief Get current scope
   * \see Scope
   * */
  Scope *get_current_scope();

  /**
   * \brief create_ty a new scope
   * \see Scope
   * */
  Scope *push_scope();

  /**
   * \brief Set the current scope
   * \see Scope
   * */
  void push_scope(Scope *);

  /**
   * \brief Pop the current scope
   * \see Scope
   * */
  Scope *pop_scope();
  [[nodiscard]] DIScope *get_current_di_scope() const;
  void push_di_scope(DIScope *scope);
  void pop_di_scope();

  /**
   * \brief Add a named ASTNode so that others can loop it up using CompilerSession::get
   * */
  void add(const str &name, ASTBase *value);

  /**
   * \brief Register a variable
   * */
  void set(const str &name, ASTBase *value);

  /**
   * \brief look up the variable table in the current and parent scopes
   * \details This function starts by searching the current scope. If the target is not found in current scope,
   * search the parent scope, repeat the process until found. Return nullptr if not found in all visible scopes.
   * */
  ASTBase *get(const str &name);

  /**
   * \brief Register a type declaration
   * \note Registers only the original type declaration or a forward declaration,
   *    use CompilerSession::add_type_accessor() to register a type accessor
   * */
  void add_type_decl(const str &name, Decl *value);

  /**
   * \brief Look up type table
   * \param name typename
   * \note Returns only the original type declaration or a forward declaration, use CompilerSession::get_type_accessor()
   * to get a type accessor
   */
  Decl *get_type_decl(const str &name);

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
  void add_function(FunctionDecl *func);
  vector<FunctionDecl *> get_functions(const str &name);
  [[nodiscard]] Loop *get_current_loop() const;
  void set_current_loop(Loop *);
  [[nodiscard]] DIFile *get_di_file() const;
  [[nodiscard]] DICompileUnit *get_di_cu() const;
  void set_current_debug_location(size_t l, size_t c);

  const str &get_filename() const;

public:
  str _filename = "";
  IRBuilder<> *_builder = nullptr; /// IR builder
  DIBuilder *_di_builder = nullptr; /// Debug information builder
  Token *_current_token = nullptr; /// Used for error messages

private:
  umap<str, Decl *> _type_decls{};
  umap<str, TypeAccessor *> _type_accessors{};

  LLVMContext *_context = nullptr;
  Module *_module = nullptr;
  vector<Scope *> _scope{}; // TODO: use tree for scope
  vector<DIScope *> _di_scope{};
  std::unique_ptr<FunctionPassManager> _fpm{};
  std::unique_ptr<PassManager> _mpm{};
  TargetMachine *_target_machine = nullptr;
  DICompileUnit *_di_cu = nullptr;
  DIFile *_di_file = nullptr;
  FunctionTable *_function_table = nullptr;

  /**
   * The control flow in current scope, used by break and continue
   * */
  Loop *_current_loop = nullptr;
  SourceManager *_source_manager = nullptr;

private:
  void initialize_scope();
  void init_llvm();
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
