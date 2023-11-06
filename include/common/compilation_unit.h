#ifndef __TAN_COMMON_COMPILATION_UNIT_H__
#define __TAN_COMMON_COMPILATION_UNIT_H__

#include "base.h"
#include "common/dependency_graph.h"

namespace tanlang {

class ASTBase;
class Program;
class SourceManager;

// TODO: rename to FileParseResult
class CompilationUnit {
public:
  CompilationUnit() = delete;
  CompilationUnit(SourceFile *src, SourceManager *sm, Program *program);
  ~CompilationUnit();

  str filename() const;
  SourceManager *source_manager() const;
  Program *ast() const;

private:
  SourceFile *_src = nullptr;
  Program *_program = nullptr;
  SourceManager *_sm = nullptr;
};

} // namespace tanlang

#endif // __TAN_COMMON_COMPILATION_UNIT_H__
