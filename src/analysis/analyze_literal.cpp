#include "src/analysis/analyzer_impl.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/expr.h"
#include "token.h"

using namespace tanlang;

void AnalyzerImpl::analyze_string_literal(ASTBase *_p) {
  auto p = ast_must_cast<StringLiteral>(_p);
  p->set_value(p->get_token_str());
  p->set_type(ASTType::Create(_cs, Ty::STRING));
}

void AnalyzerImpl::analyze_char_literal(ASTBase *_p) {
  auto p = ast_must_cast<CharLiteral>(_p);

  p->set_type(ASTType::Create(_cs, Ty::CHAR, {}));
  p->set_value(static_cast<uint8_t>(p->get_token_str()[0]));
}

void AnalyzerImpl::analyze_integer_literal(ASTBase *_p) {
  auto p = ast_must_cast<IntegerLiteral>(_p);
  auto tyty = Ty::INT;
  if (p->get_token()->is_unsigned) {
    tyty = TY_OR(tyty, Ty::UNSIGNED);
  }
  p->set_type(ASTType::Create(_cs, tyty));
}

void AnalyzerImpl::analyze_float_literal(ASTBase *_p) {
  auto p = ast_must_cast<FloatLiteral>(_p);
  p->set_type(ASTType::Create(_cs, Ty::FLOAT));
}

void AnalyzerImpl::analyze_array_literal(ASTBase *_p) {
  auto p = ast_must_cast<ArrayLiteral>(_p);

  // TODO: restrict array element type to be the same
  vector<ASTType *> sub_tys{};
  auto elements = p->get_elements();
  sub_tys.reserve(elements.size());
  std::for_each(elements.begin(), elements.end(), [&sub_tys, this](Expr *e) {
    analyze(e);
    sub_tys.push_back(e->get_type());
  });

  ASTType *ty = ASTType::Create(_cs, Ty::ARRAY, sub_tys);
  ty->_array_size = elements.size();
  p->set_type(ty);
}
