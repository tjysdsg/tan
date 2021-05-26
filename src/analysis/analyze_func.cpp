#include "src/analysis/analyzer_impl.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "compiler_session.h"
#include "src/ast/ast_func.h"

using namespace tanlang;

void AnalyzerImpl::analyze_func_call(const ParsableASTNodePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);

  std::vector<ASTNodePtr> args;
  args.reserve(p->get_children_size());
  for (const auto &c : p->get_children()) {
    args.push_back(ast_must_cast<ASTNode>(c));
  }

  p->get_children().clear();
  p->append_child(ASTFunction::GetCallee(_cs, p->get_data<str>(), args));
  np->_ty = _h.get_ty(p->get_child_at(0));
}

void AnalyzerImpl::analyze_func_decl(const ParsableASTNodePtr &p) {
  /// NOTE: children will not be automatically parsed for function declaration

  ASTFunctionPtr np = ast_must_cast<ASTFunction>(p);

  /// add to external function table
  if (np->_is_public || np->_is_external) {
    CompilerSession::AddPublicFunction(_cs->_filename, np);
  }
  /// ...and to the internal function table
  _cs->add_function(np);

  /// analyze return type
  resolve_ty(p->get_child_at<ASTTy>(0));

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
