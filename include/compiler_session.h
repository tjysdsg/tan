#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "src/ast/scope.h"

namespace tanlang {

class CompilerSession {
 public:
  std::unique_ptr<LLVMContext> _context;
  std::unique_ptr<IRBuilder<>> _builder;
  std::unique_ptr<Module> _module;
  std::vector<std::shared_ptr<Scope>> _scope;

 public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;

  CompilerSession();
  explicit CompilerSession(const std::string &module_name);
  std::shared_ptr<Scope> get_current_scope();
  std::shared_ptr<Scope> push_scope();
  std::shared_ptr<Scope> pop_scope();
  void add(const std::string &name, Value *value);
  void set(const std::string &name, Value *value);
  Value *get(const std::string &name);

 private:
  bool _is_jit_enabled = false;
  // ExecutionSession _execution_session;
  // RTDyldObjectLinkingLayer _object_layer;
  // IRCompileLayer _compile_layer;
  // DataLayout _data_layout;
  // MangleAndInterner _mangle;
  // ThreadSafeContext _ctx;
  // JITDylib _main_jd;
};

}

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
