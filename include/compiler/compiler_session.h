#ifndef TAN_INCLUDE_COMPILER_SESSION_H_
#define TAN_INCLUDE_COMPILER_SESSION_H_
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
  CompilerSession(const str &module_name);

public:
  SourceManager *get_source_manager() const;
  void set_source_manager(SourceManager *sm);
  const str &get_filename() const;

public:
  str _filename = "";
  SourceManager *_sm = nullptr;
};

} // namespace tanlang

#endif /*TAN_INCLUDE_COMPILER_SESSION_H_*/
