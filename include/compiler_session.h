#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"

namespace tanlang {

struct Scope;
class ASTNode;
using ASTNodePtr = std::shared_ptr<ASTNode>;
class ASTFunction;
using ASTFunctionPtr = std::shared_ptr<ASTFunction>;
class FunctionTable;
using FunctionTablePtr = std::shared_ptr<FunctionTable>;
class ASTLoop;

/**
 * \class CompilerSession
 * \brief Wraps all LLVM classes used for code generation
 * */
class CompilerSession final {
public:
  static void AddPublicFunction(const std::string &filename, ASTNodePtr func);
  static std::vector<ASTFunctionPtr> GetPublicFunctions(const std::string &filename);

private:
  /**
   * \brief Function table for each source files
   * \details filename -> (function name -> FunctionTable)
   * */
  static inline std::unordered_map<std::string, FunctionTablePtr> public_func{};

public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;
  CompilerSession(const std::string &module_name, TargetMachine *target_machine);
  ~CompilerSession();

public:
  /**
   * \brief Get current scope
   * \see Scope
   * */
  std::shared_ptr<Scope> get_current_scope();

  /**
   * \brief Create a new scope
   * \see Scope
   * */
  std::shared_ptr<Scope> push_scope();

  /**
   * \brief Set the current scope
   * \see Scope
   * */
  void push_scope(std::shared_ptr<Scope>);

  /**
   * \brief Pop the current scope
   * \see Scope
   * */
  std::shared_ptr<Scope> pop_scope();
  DIScope *get_current_di_scope() const;
  void push_di_scope(DIScope *scope);
  void pop_di_scope();
  void set_code_block(BasicBlock *block);
  [[nodiscard]] BasicBlock *get_code_block() const;

  /**
   * \brief Add a named ASTNode so that others can loop it up using CompilerSession::get
   * */
  void add(const std::string &name, std::shared_ptr<ASTNode> value);

  /**
   * \brief Set a named ASTNode
   * */
  void set(const std::string &name, std::shared_ptr<ASTNode> value);

  /**
   * \brief Get a named ASTNode that is visible to the current scope
   * \details This function starts by searching the current scope. If the target is not found in current scope,
   * search the parent scope, repeat the process until found. Return nullptr if not found in all visible scopes.
   * */
  std::shared_ptr<ASTNode> get(const std::string &name);
  LLVMContext *get_context();
  std::unique_ptr<IRBuilder<>> &get_builder();
  std::unique_ptr<DIBuilder> &get_di_builder();
  std::unique_ptr<Module> &get_module();
  void emit_object(const std::string &filename);

  /**
   * \brief Get the size of a pointer on the current machine
   * \details This is equivalent as llvm::TargetMachine->PointerSizeInBits()
   * */
  unsigned get_ptr_size() const;

  /**
   * \brief Add a function AST to the current file's function table
   * \details This will not add anything to the public function table, to do that,
   * call CompilerSession::AddPublicFunction
   * */
  void add_function(ASTNodePtr func);
  std::vector<ASTFunctionPtr> get_functions(const std::string &name);
  std::shared_ptr<ASTLoop> get_current_loop() const;
  void set_current_loop(std::shared_ptr<ASTLoop>);

public:
  DIFile *get_di_file() const;
  DICompileUnit *get_di_cu() const;
  void set_current_debug_location(size_t l, size_t c);

private:
  std::unique_ptr<LLVMContext> _context;
  std::unique_ptr<IRBuilder<>> _builder;
  std::unique_ptr<Module> _module;
  std::vector<std::shared_ptr<Scope>> _scope{};
  std::vector<DIScope *> _di_scope{};
  std::unique_ptr<FunctionPassManager> _fpm{};
  std::unique_ptr<PassManager> _mpm{};
  TargetMachine *_target_machine = nullptr;

  std::unique_ptr<DIBuilder> _di_builder{}; /// Debug information builder
  DICompileUnit *_di_cu = nullptr;
  DIFile *_di_file = nullptr;
  FunctionTablePtr _function_table = nullptr;

  /**
   * The control flow in current scope, used by break and continue
   * */
  std::shared_ptr<ASTLoop> _current_loop = nullptr;

private:
  void initialize_scope();
  void init_llvm();
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
