#include "common/compilation_unit.h"
#include "source_file/tokenized_source_file.h"
#include "ast/stmt.h"

namespace tanlang {

CompilationUnit::CompilationUnit(SourceFile *src, TokenizedSourceFile *tsrc, Program *program)
    : _src(src), _program(program), _tsrc(tsrc) {}

str CompilationUnit::filename() const { return _tsrc->get_filename(); }

TokenizedSourceFile *CompilationUnit::src() const { return _tsrc; }

Program *CompilationUnit::ast() const { return _program; }

CompilationUnit::~CompilationUnit() {
  delete _program;
  delete _tsrc;
  delete _src;
}

} // namespace tanlang
