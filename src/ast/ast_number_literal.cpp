#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTNumberLiteral::codegen(CompilerSession *cs) { return _ty->get_llvm_value(cs); }

size_t ASTNumberLiteral::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

void ASTNumberLiteral::resolve() {
  Ty t = Ty::INVALID;
  if (_is_float) {
    t = Ty::FLOAT;
    _ty = ASTTy::Create(t, std::vector<ASTNodePtr>());
    _ty->_default_value = _fvalue;
  } else if (_is_unsigned) {
    t = TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED);
    _ty = ASTTy::Create(t, std::vector<ASTNodePtr>());
    _ty->_default_value = static_cast<uint64_t>(_ivalue);
  } else {
    t = TY_OR(Ty::INT, Ty::BIT32);
    _ty = ASTTy::Create(t, std::vector<ASTNodePtr>());
    _ty->_default_value = static_cast<uint64_t>(_ivalue);
  }
}

bool tanlang::ASTNumberLiteral::is_float() const { return _is_float; }

str ASTNumberLiteral::to_string(bool print_prefix) const {
  str ret = "";
  if (print_prefix) {
    ret += ASTLiteral::to_string(print_prefix) + " ";
  }
  if (_is_float) { ret += std::to_string(_fvalue); } else { ret += std::to_string(_ivalue); }
  return ret;
}

ASTNumberLiteral::ASTNumberLiteral(int value, size_t token_index, bool is_unsigned) : ASTLiteral(ASTType::NUM_LITERAL,
    op_precedence[ASTType::NUM_LITERAL],
    0,
    nullptr,
    token_index) {
  _ivalue = value;
  _is_float = false;
  _is_unsigned = is_unsigned;
  resolve();
}

ASTNumberLiteral::ASTNumberLiteral(size_t value, size_t token_index, bool is_unsigned)
    : ASTLiteral(ASTType::NUM_LITERAL, op_precedence[ASTType::NUM_LITERAL], 0, nullptr, token_index) {
  _ivalue = static_cast<int>(value);
  _is_float = false;
  _is_unsigned = is_unsigned;
  resolve();
}

ASTNumberLiteral::ASTNumberLiteral(float value, size_t token_index) : ASTLiteral(ASTType::NUM_LITERAL,
    op_precedence[ASTType::NUM_LITERAL],
    0,
    nullptr,
    token_index) {
  _fvalue = value;
  _is_float = true;
  resolve();
}

ASTNumberLiteral::ASTNumberLiteral(const str &str, bool is_float, Token *token, size_t token_index)
    : ASTLiteral(ASTType::NUM_LITERAL, op_precedence[ASTType::NUM_LITERAL], 0, token, token_index) {
  _is_float = is_float;
  if (is_float) { _fvalue = std::stof(str); }
  else { _ivalue = std::stoi(str); }
  _is_unsigned = token->is_unsigned;
  resolve();
}

} // namespace tanlang
