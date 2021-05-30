#include "src/analysis/analyzer_impl.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"
#include "compiler.h"

using namespace tanlang;

void AnalyzerImpl::analyze_import(const ASTBasePtr &p) {
  // TODO: determine whether to use class field or child ASTNode to store imported filename
  // auto rhs = p->get_child_at(0);
  // str file = std::get<str>(rhs->_value);
  str file = p->get_data<str>();
  auto imported = Compiler::resolve_import(_cs->_filename, file);
  if (imported.empty()) {
    report_error(p, "Cannot import: " + file);
  }

  /// it might be already parsed
  vector<ASTFunctionPtr> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  if (imported_functions.empty()) {
    Compiler::ParseFile(imported[0]);
    imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  }
  for (ASTFunctionPtr f: imported_functions) {
    _cs->add_function(f);
    p->append_child(f);
  }
}
