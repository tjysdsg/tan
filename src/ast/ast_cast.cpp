#include "src/ast/ast_cast.h"
#include "src/ast/ast_ty.h"
#include "src/type_system.h"
#include "src/common.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

size_t ASTCast::led(const ASTNodePtr &left) {
  _end_index = _start_index + 1; /// skip operator
  _children.emplace_back(left); /// lhs
  _ty = ast_cast<ASTTy>(_parser->parse<ASTType::TY>(_end_index, true));
  _children.push_back(_ty);
  _dominant_idx = this->get_dominant_idx();
  return _end_index;
}

Value *ASTCast::codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  auto *dest_type = _children[1]->to_llvm_type(cs);
  Value *val = lhs->codegen(cs);
  Value *ret = nullptr;
  if (lhs->is_lvalue()) { val = builder->CreateLoad(val); }
  val = TypeSystem::ConvertTo(cs, dest_type, val, false);
  if (lhs->is_lvalue()) {
    ret = create_block_alloca(builder->GetInsertBlock(), dest_type);
    builder->CreateStore(val, ret);
  } else { ret = val; }
  return ret;
}

size_t ASTCast::get_dominant_idx() const { return 1; }

bool ASTCast::is_typed() const { return true; }

bool ASTCast::is_lvalue() const {
  TAN_ASSERT(_children.size() == 2);
  return _children[0]->is_lvalue();
}

ASTCast::ASTCast(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::CAST;
  _lbp = op_precedence[_type];
}

} // namespace tanlang
