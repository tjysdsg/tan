#include "common/compilation_unit.h"
#include "source_file/source_manager.h"
#include "ast/stmt.h"

namespace tanlang {

CompilationUnit::CompilationUnit(SourceFile *src, SourceManager *sm, Program *program)
    : _src(src), _program(program), _sm(sm) {}

str CompilationUnit::filename() const { return _sm->get_filename(); }

SourceManager *CompilationUnit::source_manager() const { return _sm; }

Program *CompilationUnit::ast() const { return _program; }

CompilationUnit::~CompilationUnit() {
  delete _program;
  delete _sm;
  delete _src;
}

} // namespace tanlang
