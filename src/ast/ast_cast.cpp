#include "src/ast/ast_cast.h"
#include "src/type_system.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/common.h"
#include "token.h"

namespace tanlang {

ASTCast::ASTCast(Token *token, size_t token_index) : ASTInfixBinaryOp(token, token_index) {
  _type = ASTType::CAST;
  _lbp = op_precedence[_type];
}

Value *ASTCast::codegen(CompilerSession *cm) {
  cm->set_current_debug_location(_token->l, _token->c);
  auto lhs = _children[0];
  auto *dest_type = _children[1]->to_llvm_type(cm);
  Value *val = lhs->codegen(cm);
  Value *ret = val;
  if (lhs->is_lvalue()) { val = cm->get_builder()->CreateLoad(val); }
  val = TypeSystem::ConvertTo(cm, dest_type, val, false);
  if (lhs->is_lvalue()) {
    ret = create_block_alloca(cm->get_builder()->GetInsertBlock(), dest_type);
    cm->get_builder()->CreateStore(val, ret);
  } else { ret = val; }
  return ret;
}

size_t ASTCast::led(const ASTNodePtr &left, Parser *parser) {
  _end_index = _start_index + 1; /// skip operator
  _children.emplace_back(left); /// lhs
  auto ty = ast_cast<ASTTy>(parser->parse<ASTType::TY>(_end_index, true));
  _children.push_back(ty);
  _dominant_idx = this->get_dominant_idx();
  return _end_index;
}

size_t ASTCast::get_dominant_idx() const { return 1; }

bool ASTCast::is_typed() const { return true; }

bool ASTCast::is_lvalue() const {
  assert(_children.size() == 2);
  return _children[0]->is_lvalue();
}

std::string ASTCast::get_type_name() const {
  assert(_children.size() == 2);
  return _children[1]->get_type_name();
}

std::shared_ptr<ASTTy> ASTCast::get_ty() const {
  assert(_children.size() == 2);
  return _children[1]->get_ty();
}

llvm::Type *ASTCast::to_llvm_type(CompilerSession *cm) const {
  assert(_children.size() == 2);
  return _children[1]->to_llvm_type(cm);
}

} // namespace tanlang
