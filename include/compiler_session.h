#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "src/ast/scope.h"

namespace tanlang {

class CompilerSession final {
public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;

  CompilerSession(const std::string &module_name);
  ~CompilerSession() = default;

  std::shared_ptr<Scope> get_current_scope();
  std::shared_ptr<Scope> push_scope();
  std::shared_ptr<Scope> pop_scope();
  void set_code_block(BasicBlock *block);
  [[nodiscard]] BasicBlock *get_code_block() const;
  void add(const std::string &name, std::shared_ptr<ASTNode> value);
  void set(const std::string &name, std::shared_ptr<ASTNode> value);
  std::shared_ptr<ASTNode> get(const std::string &name);
  LLVMContext *get_context();
  std::unique_ptr<IRBuilder<>> &get_builder();
  std::unique_ptr<Module> &get_module();
  std::unique_ptr<FunctionPassManager> &get_function_pass_manager();

private:
  std::unique_ptr<LLVMContext> _context;
  std::unique_ptr<IRBuilder<>> _builder;
  std::unique_ptr<Module> _module;
  std::vector<std::shared_ptr<Scope>> _scope{};
  std::unique_ptr<FunctionPassManager> _fpm{};

private:
  void initialize_scope();
  void init_llvm_pass();
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
