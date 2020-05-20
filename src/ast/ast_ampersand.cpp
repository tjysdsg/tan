#include "src/ast/ast_ampersand.h"
#include "src/ast/ast_ty.h"
#include "src/analysis/analysis.h"
#include "compiler_session.h"
#include "src/common.h"
#include "parser.h"

using namespace tanlang;

size_t ASTAmpersand::nud() {
  _type = ASTType::ADDRESS_OF;
  _rbp = op_precedence[_type];
  _end_index = _start_index + 1; /// skip '&'
  auto rhs = _parser->next_expression(_end_index, _rbp);
  _children.push_back(rhs);
  if (!(_ty = Analyzer::get_ty(rhs))) { error("Invalid operand"); }
  _ty = Analyzer::get_ptr_to(_ty);
  return _end_index;
}

/// set type as invalid first, since we do not know if this is a 'binary and' or 'get address of'
ASTAmpersand::ASTAmpersand(Token *token, size_t token_index) : ASTNode(ASTType::INVALID, 0, 0, token, token_index) {
  _is_typed = true;
  _is_valued = true;
}

Value *ASTAmpersand::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  if (_type == ASTType::ADDRESS_OF) {
    auto *val = _children[0]->codegen(cs);
    if (Analyzer::is_lvalue(_children[0])) { /// lvalue, the val itself is a pointer to real value
      _llvm_value = val;
    } else { /// rvalue, create an anonymous variable, and get address of it
      _llvm_value = create_block_alloca(builder->GetInsertBlock(), val->getType(), 1, "anonymous");
      builder->CreateStore(val, _llvm_value);
    }
  } else if (_type == ASTType::BAND) {
    // TODO: codegen for binary and
    TAN_ASSERT(false);
  } else { TAN_ASSERT(false); }
  return _llvm_value;
}

ASTAmpersandPtr ASTAmpersand::CreateAddressOf(ASTNodePtr n) {
  auto ret = std::make_shared<ASTAmpersand>(nullptr, 0);
  ret->_type = ASTType::ADDRESS_OF;
  ret->_rbp = op_precedence[ASTType::ADDRESS_OF];
  ret->_children.push_back(n);
  ret->_ty = Analyzer::get_ptr_to(Analyzer::get_ty(n));
  return ret;
}

// TODO
size_t ASTAmpersand::led(const ASTNodePtr &) { TAN_ASSERT(false); }
