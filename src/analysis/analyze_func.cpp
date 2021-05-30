#include "src/analysis/analyzer_impl.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "src/ast/decl.h"
#include "compiler_session.h"

using namespace tanlang;

void AnalyzerImpl::analyze_func_call(const ASTBasePtr &_p) {
  auto p = ast_must_cast<FunctionCall>(_p);

  for (const auto &a: p->_args) {
    analyze(a);
  }

  FunctionDeclPtr callee = FunctionDecl::GetCallee(_cs, p->get_name(), p->_args);
  p->_callee = callee;
  p->set_type(copy_ty(callee->get_ret_ty()));
}

void AnalyzerImpl::analyze_func_decl(const ASTBasePtr &_p) {
  FunctionDeclPtr p = ast_must_cast<FunctionDecl>(_p);

  /// add to external function table
  if (p->is_public() || p->is_external()) {
    CompilerSession::AddPublicFunction(_cs->_filename, p);
  }
  /// ...and to the internal function table
  _cs->add_function(p);

  /// analyze return type
  analyze(p->get_ret_ty());

  _cs->push_scope(); /// new scope

  /// analyze args
  size_t n = p->get_n_args();
  for (size_t i = 0; i < n; ++i) {
    analyze(p->get_arg_type(i)); /// args will be added to the scope here
  }

  /// function body
  if (!p->is_external()) {
    analyze(p->get_body());
  }

  _cs->pop_scope(); /// pop scope
}
