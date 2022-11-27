#include "compiler/compiler_session.h"
#include "ast/ast_base.h"
#include "compiler/compiler.h"

using namespace tanlang;

CompilerSession::CompilerSession(const str &module_name) : _filename(module_name) {}

SourceManager *CompilerSession::get_source_manager() const { return _sm; }

void CompilerSession::set_source_manager(SourceManager *sm) { _sm = sm; }

const str &CompilerSession::get_filename() const { return _filename; }
