#include "src/ast/ast_ty.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

Value *ASTNumberLiteral::_codegen(CompilerSession *cs) { return _ty->get_llvm_value(cs); }

size_t ASTNumberLiteral::nud() {
  _end_index = _start_index + 1;
  return _end_index;
}

void ASTNumberLiteral::resolve() {
  Ty t;
  if (_is_float) {
    t = Ty::FLOAT;
    _ty = ASTTy::Create(t, vector<ASTNodePtr>());
    _ty->_default_value = _fvalue;
  } else if (_is_unsigned) {
    t = TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED);
    _ty = ASTTy::Create(t, vector<ASTNodePtr>());
    _ty->_default_value = static_cast<uint64_t>(_ivalue);
  } else {
    t = TY_OR(Ty::INT, Ty::BIT32);
    _ty = ASTTy::Create(t, vector<ASTNodePtr>());
    _ty->_default_value = static_cast<uint64_t>(_ivalue);
  }
}

str ASTNumberLiteral::to_string(bool print_prefix) {
  str ret = "";
  if (print_prefix) {
    ret += ASTLiteral::to_string(print_prefix) + " ";
  }
  if (_is_float) { ret += std::to_string(_fvalue); } else { ret += std::to_string(_ivalue); }
  return ret;
}

} // namespace tanlang
