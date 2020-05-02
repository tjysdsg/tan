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

class CompilerSession final {
public:
  static void AddPublicFunction(const std::string &filename, ASTNodePtr func);
  static std::vector<ASTFunctionPtr> GetPublicFunctions(const std::string &filename);

private:
  /// filename -> (function name -> FunctionTable)
  static std::unordered_map<std::string, FunctionTablePtr> public_func;

public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;
  CompilerSession(const std::string &module_name, TargetMachine *target_machine);
  ~CompilerSession();

public:
  std::shared_ptr<Scope> get_current_scope();
  std::shared_ptr<Scope> push_scope();
  void push_scope(std::shared_ptr<Scope>);
  std::shared_ptr<Scope> pop_scope();
  DIScope *get_current_di_scope() const;
  void push_di_scope(DIScope *scope);
  void pop_di_scope();
  void set_code_block(BasicBlock *block);
  [[nodiscard]] BasicBlock *get_code_block() const;
  void add(const std::string &name, std::shared_ptr<ASTNode> value);
  void set(const std::string &name, std::shared_ptr<ASTNode> value);
  std::shared_ptr<ASTNode> get(const std::string &name);
  LLVMContext *get_context();
  std::unique_ptr<IRBuilder<>> &get_builder();
  std::unique_ptr<DIBuilder> &get_di_builder();
  std::unique_ptr<Module> &get_module();
  void emit_object(const std::string &filename);
  unsigned get_ptr_size() const;
  void add_function(ASTNodePtr func);
  std::vector<ASTFunctionPtr> get_functions(const std::string &name);

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

  /// debug information
  std::unique_ptr<DIBuilder> _di_builder{};
  DICompileUnit *_di_cu = nullptr;
  DIFile *_di_file = nullptr;
  FunctionTablePtr _function_table = nullptr;

private:
  void initialize_scope();
  void init_llvm();
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
