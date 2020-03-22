#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "src/ast/scope.h"

namespace tanlang {

class CompilerSession final {
public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;

  CompilerSession();

  ~CompilerSession() {

  };
  explicit CompilerSession(const std::string &module_name);
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

private:
  std::unique_ptr<LLVMContext> _context;
  std::unique_ptr<IRBuilder<>> _builder;
  std::unique_ptr<Module> _module;
  std::vector<std::shared_ptr<Scope>> _scope;

private:
  void initialize_scope();

  // jit related
public:
  std::unique_ptr<ExecutionSession> &get_execution_session() { return _execution_session; }

  std::unique_ptr<MangleAndInterner> &get_mangle() { return _mangle; }

  std::unique_ptr<ThreadSafeContext> &get_threadsafe_context() { return _ctx; }

  std::unique_ptr<IRCompileLayer> &get_compile_layer() { return _compile_layer; }

  [[nodiscard]] bool is_jit_enabled() const { return _is_jit_enabled; }

  CompilerSession(std::unique_ptr<IRBuilder<>> builder,
                  std::unique_ptr<Module> module,
                  std::unique_ptr<ExecutionSession> execution_session,
                  std::unique_ptr<RTDyldObjectLinkingLayer> object_layer,
                  std::unique_ptr<IRCompileLayer> compile_layer,
                  std::unique_ptr<DataLayout> data_layout,
                  std::unique_ptr<MangleAndInterner> mangle,
                  std::unique_ptr<ThreadSafeContext> ctx);

private:
  bool _is_jit_enabled = false;
  std::unique_ptr<ExecutionSession> _execution_session{};
  std::unique_ptr<RTDyldObjectLinkingLayer> _object_layer{};
  std::unique_ptr<IRCompileLayer> _compile_layer{};
  std::unique_ptr<DataLayout> _data_layout{};
  std::unique_ptr<MangleAndInterner> _mangle{};
  std::unique_ptr<ThreadSafeContext> _ctx{};
};

}

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
