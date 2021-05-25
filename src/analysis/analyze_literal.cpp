#include "src/analysis/analyzer_impl.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "token.h"

using namespace tanlang;

void AnalyzerImpl::analyze_char_literal(ParsableASTNodePtr &p) {
  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  np->_ty = create_ty(_cs, Ty::CHAR, {});
  np->set_data(static_cast<uint64_t>(p->get_token()->value[0]));
  np->_ty->_default_value = p->get_data<uint64_t>();
}

void AnalyzerImpl::analyze_num_literal(ParsableASTNodePtr &p) {
  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  if (p->get_token()->type == TokenType::INT) {
    auto tyty = Ty::INT;
    if (p->get_token()->is_unsigned) {
      tyty = TY_OR(tyty, Ty::UNSIGNED);
    }
    np->_ty = create_ty(_cs, tyty);
  } else if (p->get_token()->type == TokenType::FLOAT) {
    np->_ty = create_ty(_cs, Ty::FLOAT);
  }
}

void AnalyzerImpl::analyze_array_literal(ParsableASTNodePtr &p) {
  ASTNodePtr np = _h.try_convert_to_ast_node(p);

  vector<ASTNodePtr> sub_tys{};
  sub_tys.reserve(p->get_children_size());
  std::for_each(p->get_children().begin(), p->get_children().end(), [&sub_tys](const ASTNodePtr &e) {
    sub_tys.push_back(e->_ty);
  });
  np->_ty = create_ty(_cs, Ty::ARRAY, sub_tys);
}
