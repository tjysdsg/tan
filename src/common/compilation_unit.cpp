#include "common/compilation_unit.h"
#include "source_file/source_manager.h"

namespace tanlang {

CompilationUnit::CompilationUnit(Program *program, SourceManager *sm) : _program(program), _sm(sm) {}

str CompilationUnit::filename() const { return _sm->get_filename(); }

SourceManager *CompilationUnit::source_manager() const { return _sm; }

Program *CompilationUnit::ast() const { return _program; }

} // namespace tanlang
