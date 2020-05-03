#include "src/ast/ast_ampersand.h"
#include "src/ast/ast_ty.h"
#include "parser.h"
#include "compiler_session.h"
#include "src/common.h"

namespace tanlang {

size_t ASTAmpersand::nud() {
  _type = ASTType::ADDRESS_OF;
  _rbp = op_precedence[_type];
  _end_index = _start_index + 1; /// skip '&'
  auto rhs = _parser->next_expression(_end_index, _rbp);
  _children.push_back(rhs);
  _ty = rhs->get_ty()->get_ptr_to();
  return _end_index;
}

/// set type as invalid first, since we do not know if this is a 'binary and' or 'get address of'
ASTAmpersand::ASTAmpersand(Token *token, size_t token_index) : ASTNode(ASTType::INVALID, 0, 0, token, token_index) {}

Value *ASTAmpersand::codegen(CompilerSession *cs) {
  if (_type == ASTType::ADDRESS_OF) {
    auto *val = _children[0]->codegen(cs);
    if (_children[0]->is_lvalue()) { /// lvalue, the val itself is a pointer to real value
      _llvm_value = val;
    } else { /// rvalue, create an anonymous variable, and get address of it
      _llvm_value = create_block_alloca(cs->get_builder()->GetInsertBlock(), val->getType(), "anonymous");
      cs->get_builder()->CreateStore(val, _llvm_value);
    }
  } else if (_type == ASTType::BAND) {
    // TODO: codegen for binary and
    TAN_ASSERT(false);
  } else { TAN_ASSERT(false); }
  return _llvm_value;
}

// TODO
size_t ASTAmpersand::led(const ASTNodePtr &) { TAN_ASSERT(false); }

bool ASTAmpersand::is_typed() const { return true; }

bool ASTAmpersand::is_lvalue() const { return false; }

} // namespace tanlang
