#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
#include "src/llvm_include.h"
#include "base.h"
#include "ast/fwd.h"

namespace tanlang {

/**
 * \class CompilerSession
 * \brief Wraps all LLVM classes used for code generation
 * */
class CompilerSession final {
public:
  CompilerSession &operator=(const CompilerSession &) = delete;
  CompilerSession(const CompilerSession &) = delete;
  CompilerSession() = delete;
  CompilerSession(const str &module_name, TargetMachine *target_machine);
  ~CompilerSession();

  /// avoid creating duplicated llvm::Type and llvm::Metadata
  umap<Type *, llvm::Type *> llvm_type_cache{};
  umap<Type *, llvm::Metadata *> llvm_metadata_cache{};

public:
  SourceManager *get_source_manager() const;
  void set_source_manager(SourceManager *sm);

  [[nodiscard]] DIScope *get_current_di_scope() const;
  void push_di_scope(DIScope *scope);
  void pop_di_scope();

  LLVMContext *get_context();
  Module *get_module();
  void emit_object(const str &filename);
  unsigned get_ptr_size() const;

  [[nodiscard]] DIFile *get_di_file() const;
  [[nodiscard]] DICompileUnit *get_di_cu() const;
  void set_current_debug_location(size_t l, size_t c);

  const str &get_filename() const;

public:
  str _filename = "";
  SourceManager *_sm = nullptr;
  IRBuilder<> *_builder = nullptr; /// IR builder
  DIBuilder *_di_builder = nullptr; /// Debug information builder

private:
  LLVMContext *_context = nullptr;
  Module *_module = nullptr;
  vector<DIScope *> _di_scope{};
  TargetMachine *_target_machine = nullptr;
  DICompileUnit *_di_cu = nullptr;
  DIFile *_di_file = nullptr;
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
