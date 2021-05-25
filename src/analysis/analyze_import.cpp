#include "src/analysis/analyzer_impl.h"
#include "src/analysis/type_system.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "src/common.h"
#include "compiler_session.h"
#include "compiler.h"

using namespace tanlang;

void AnalyzerImpl::analyze_import(ParsableASTNodePtr &p) {
  // TODO: determine whether to use class field or child ASTNode to store imported filename
  // auto rhs = p->get_child_at(0);
  // str file = std::get<str>(rhs->_value);
  str file = p->get_data<str>();
  auto imported = Compiler::resolve_import(_cs->_filename, file);
  if (imported.empty()) { report_error(_cs, p, "Cannot import: " + file); }

  /// it might be already parsed
  vector<ASTFunctionPtr> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  if (imported_functions.empty()) {
    Compiler::ParseFile(imported[0]);
    imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
  }
  for (auto &f: imported_functions) {
    _cs->add_function(f);
    p->append_child(f);
  }
}