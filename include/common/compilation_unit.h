#ifndef __TAN_COMMON_COMPILATION_UNIT_H__
#define __TAN_COMMON_COMPILATION_UNIT_H__

#include "base.h"
#include "common/dependency_graph.h"

namespace tanlang {

class ASTBase;
class Program;
class TokenizedSourceFile;

// TODO: rename to FileParseResult or remove this class
class CompilationUnit {
public:
  CompilationUnit() = delete;
  CompilationUnit(SourceFile *src, TokenizedSourceFile *tsrc, Program *program);
  ~CompilationUnit();

  str filename() const;
  TokenizedSourceFile *src() const;
  Program *ast() const;

private:
  SourceFile *_src = nullptr;
  Program *_program = nullptr;
  TokenizedSourceFile *_tsrc = nullptr;
};

} // namespace tanlang

#endif // __TAN_COMMON_COMPILATION_UNIT_H__
