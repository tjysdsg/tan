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
  ASTFunctionPtr np = ast_must_cast<ASTFunction>(p);

  /// add to function table
  if (np->_is_public || np->_is_external) { CompilerSession::AddPublicFunction(_cs->_filename, np); }
  /// ...and to the internal function table
  _cs->add_function(np);

  // TODO: function type
  //  auto ret_ty = ast_create_ty(_cs);
  //  ret_ty->set_token(at(p->_end_index));
  //  ret_ty->_end_index = ret_ty->_start_index = p->_end_index;
  //  p->_end_index = parse_ty(ret_ty); /// return type
  //  p->get_child_at(0) = ret_ty;

  /// add args to scope if function body exists
  size_t n = p->get_children_size();
  size_t arg_end = n - 1 - !np->_is_external;
  for (size_t i = 1; i < arg_end; ++i) {
    if (!np->_is_external) { _cs->set_type(p->get_child_at(i)->get_data<str>(), _h.get_ty(p->get_child_at(i))); }
  }
  if (!np->_is_external) {
    /// new scope for function body
    auto f_body = np->get_child_at(n - 1);
    if (!np->_is_external) {
      f_body->set_scope(_cs->push_scope());
    }
  }
}
