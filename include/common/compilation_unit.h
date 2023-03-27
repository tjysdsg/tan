#ifndef __TAN_COMMON_COMPILATION_UNIT_H__
#define __TAN_COMMON_COMPILATION_UNIT_H__

#include "base.h"
#include "common/dependency_graph.h"

namespace tanlang {

class ASTBase;
class Program;
class SourceManager;

class CompilationUnit {
public:
  CompilationUnit() = delete;
  CompilationUnit(Program *program, SourceManager *sm);

  str filename() const;
  SourceManager *source_manager() const;
  Program *ast() const;

public:
  DependencyGraph<ASTBase *> top_level_symbol_dependency{};

private:
  Program *_program = nullptr;
  SourceManager *_sm = nullptr;
};

} // namespace tanlang

#endif // __TAN_COMMON_COMPILATION_UNIT_H__
