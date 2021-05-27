#include "src/analysis/analyzer_impl.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "token.h"

using namespace tanlang;

void AnalyzerImpl::analyze_string_literal(const ParsableASTNodePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);
  np->_ty = create_ty(_cs, Ty::STRING);
}

void AnalyzerImpl::analyze_char_literal(const ParsableASTNodePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);

  np->_ty = create_ty(_cs, Ty::CHAR, {});
  np->set_data(static_cast<uint64_t>(p->get_token()->value[0]));
  np->_ty->_default_value = p->get_data<uint64_t>();
}

void AnalyzerImpl::analyze_num_literal(const ParsableASTNodePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);

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

void AnalyzerImpl::analyze_array_literal(const ParsableASTNodePtr &p) {
  auto np = ast_must_cast<ASTNode>(p);

  // TODO: restrict array element type to be the same
  vector<ASTTyPtr> sub_tys{};
  sub_tys.reserve(p->get_children_size());
  std::for_each(p->get_children().begin(), p->get_children().end(), [&sub_tys, this](const ParsableASTNodePtr &e) {
    sub_tys.push_back(_h.get_ty(e));
  });

  ASTTyPtr ty = create_ty(_cs, Ty::ARRAY, sub_tys);
  ty->_array_size = p->get_children_size();
  np->_ty = ty;
}
