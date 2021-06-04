#include "src/analysis/analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/stmt.h"
#include "compiler_session.h"
#include "compiler.h"

using namespace tanlang;

void AnalyzerImpl::analyze_import(ASTBase *_p) {
  auto p = ast_must_cast<Import>(_p);

  str file = p->get_filename();
  auto imported = Compiler::resolve_import(_cs->_filename, file);
  if (imported.empty()) {
    report_error(p, "Cannot import: " + file);
  }

  /// it might be already parsed
  vector<FunctionDecl *> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  if (imported_functions.empty()) {
    Compiler::ParseFile(imported[0]);
    imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  }

  /// import functions
  p->set_imported_funcs(imported_functions);
  for (FunctionDecl *f: imported_functions) {
    _cs->add_function(f);
  }
}
