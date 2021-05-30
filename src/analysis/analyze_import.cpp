#include "src/analysis/analyzer_impl.h"
#include "src/ast/ast_type.h"
#include "src/ast/stmt.h"
#include "compiler_session.h"
#include "compiler.h"

using namespace tanlang;

void AnalyzerImpl::analyze_import(const ASTBasePtr &_p) {
  auto p = ast_must_cast<Import>(_p);

  str file = p->get_filename();
  auto imported = Compiler::resolve_import(_cs->_filename, file);
  if (imported.empty()) {
    report_error(p, "Cannot import: " + file);
  }

  /// it might be already parsed
  vector<FunctionDeclPtr> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  if (imported_functions.empty()) {
    Compiler::ParseFile(imported[0]);
    imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  }
  for (FunctionDeclPtr f: imported_functions) {
    _cs->add_function(f);
    p->append_child(f);
  }
}
