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
  if (np->_is_public || np->_is_external) {
    CompilerSession::AddPublicFunction(_cs->_filename, np);
  }
  /// ...and to the internal function table
  _cs->add_function(np);

  /// analyze return type
  resolve_ty(p->get_child_at<ASTType>(0));

  _cs->push_scope(); /// new scope

  /// add args to scope if function body exists
  size_t n = p->get_children_size();
  size_t n_args = np->get_n_args();
  for (size_t i = 1; i < n_args + 1; ++i) {
    ASTNodePtr child = p->get_child_at<ASTNode>(i);
    analyze(child); /// args will be added to the scope here
  }

  /// function body
  if (!np->_is_external) {
    auto body = np->get_child_at(n - 1);
    analyze(body);
  }

  _cs->pop_scope(); /// pop scope
}
