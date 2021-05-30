#include "src/ast/factory.h"
#include "src/common.h"
#include "src/ast/ast_member_access.h"
#include "intrinsic.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

/// \section Literals

ASTNodePtr ast_create_string_literal(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::STRING_LITERAL, ASTNode::OpPrecedence[ASTNodeType::STRING_LITERAL]);
  ret->_is_valued = true;
  ret->_is_typed = true;
  ret->_type = create_ty(cs, Ty::STRING);
  return ret;
}

ASTNodePtr ast_create_string_literal(CompilerSession *cs, const str &s) {
  auto ret = ast_create_string_literal(cs);
  ret->set_data(s);
  return ret;
}

ASTNodePtr ast_create_array_literal(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::ARRAY_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

static ASTNodePtr ast_create_numeric_literal() {
  auto ret = make_ptr<ASTNode>(ASTNodeType::NUM_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_numeric_literal(CompilerSession *cs, uint64_t val, bool is_unsigned) {
  auto ret = ast_create_numeric_literal();
  // TODO: bit size
  ASTTypePtr ty = nullptr;
  if (is_unsigned) {
    ty = create_ty(cs, TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED));
  } else {
    ty = create_ty(cs, TY_OR(Ty::INT, Ty::BIT32));
  }
  ty->_default_value = val;
  ret->set_data(val);
  ret->_type = ty;
  return ret;
}

ASTNodePtr ast_create_numeric_literal(CompilerSession *cs, double val) {
  auto ret = ast_create_numeric_literal();
  // TODO: float or double
  ASTTypePtr ty = create_ty(cs, Ty::FLOAT);
  ty->_default_value = val;
  ret->set_data(val);
  ret->_type = ty;
  return ret;
}

ASTNodePtr ast_create_char_literal(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::CHAR_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_char_literal(CompilerSession *cs, char c) {
  auto ret = ast_create_char_literal(cs);
  ret->set_data(static_cast<uint64_t>(c));
  return ret;
}

/// \section Ops

ASTNodePtr ast_create_cast(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::CAST, ASTNode::OpPrecedence[ASTNodeType::CAST]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_arithmetic(CompilerSession *, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::INVALID, 0);
  switch (hashed_string{op.c_str()}) {
    case "+"_hs:
      ret->set_node_type(ASTNodeType::SUM);
      break;
    case "-"_hs:
      ret->set_node_type(ASTNodeType::SUBTRACT);
      break;
    case "*"_hs:
      ret->set_node_type(ASTNodeType::MULTIPLY);
      break;
    case "/"_hs:
      ret->set_node_type(ASTNodeType::DIVIDE);
      break;
    case "%"_hs:
      ret->set_node_type(ASTNodeType::MOD);
      break;
    default:
      return nullptr;
  }
  ret->set_lbp(ASTNode::OpPrecedence[ret->get_node_type()]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_comparison(CompilerSession *, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::INVALID, 0);
  switch (hashed_string{op.c_str()}) {
    case ">"_hs:
      ret->set_node_type(ASTNodeType::GT);
      break;
    case ">="_hs:
      ret->set_node_type(ASTNodeType::GE);
      break;
    case "<"_hs:
      ret->set_node_type(ASTNodeType::LT);
      break;
    case "<="_hs:
      ret->set_node_type(ASTNodeType::LE);
      break;
    case "=="_hs:
      ret->set_node_type(ASTNodeType::EQ);
      break;
    case "!="_hs:
      ret->set_node_type(ASTNodeType::NE);
      break;
    default:
      return nullptr;
  }
  ret->set_lbp(ASTNode::OpPrecedence[ret->get_node_type()]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_assignment(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::ASSIGN, ASTNode::OpPrecedence[ASTNodeType::ASSIGN]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_member_access(CompilerSession *) {
  auto ret = make_ptr<ASTMemberAccess>(ASTNodeType::MEMBER_ACCESS, ASTNode::OpPrecedence[ASTNodeType::MEMBER_ACCESS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_return(CompilerSession *) {
  return make_ptr<ASTNode>(ASTNodeType::RET, ASTNode::OpPrecedence[ASTNodeType::RET]);
}

ASTNodePtr ast_create_not(CompilerSession *) {
  /// logical not or bitwise not
  auto ret = make_ptr<ASTNode>(ASTNodeType::INVALID, 0);
  ret->_is_valued = true;
  ret->_is_typed = true;
  return ret;
}

ASTNodePtr ast_create_ampersand(CompilerSession *) {
  /// address_of or binary and
  auto ret = make_ptr<ASTNode>(ASTNodeType::INVALID, 0);
  ret->_is_valued = true;
  ret->_is_typed = true;
  return ret;
}

ASTNodePtr ast_create_address_of(CompilerSession *, ASTBasePtr p) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::ADDRESS_OF, ASTNode::OpPrecedence[ASTNodeType::ADDRESS_OF]);
  ret->_is_valued = true;
  ret->_is_typed = true;
  ret->append_child(p);
  return ret;
}

/// \section Control flow

ASTNodePtr ast_create_if(CompilerSession *) {
  auto ret = make_ptr<ASTIf>(ASTNodeType::IF, ASTNode::OpPrecedence[ASTNodeType::IF]);
  return ret;
}

ASTNodePtr ast_create_else(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::ELSE, ASTNode::OpPrecedence[ASTNodeType::ELSE]);
  return ret;
}

ASTNodePtr ast_create_loop(CompilerSession *) {
  auto ret = make_ptr<ASTLoop>();
  return ret;
}

ASTNodePtr ast_create_break(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::BREAK, 0);
  return ret;
}

ASTNodePtr ast_create_continue(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::CONTINUE, 0);
  return ret;
}

/// \section Others

ASTNodePtr ast_create_program(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::PROGRAM, 0);
  return ret;
}

ASTNodePtr ast_create_import(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::IMPORT, 0);
  ret->_is_named = true; /// name is the file imported
  return ret;
}

ASTNodePtr ast_create_intrinsic(CompilerSession *) {
  auto ret = make_ptr<Intrinsic>();
  return ret;
}

ASTNodePtr ast_create_statement(CompilerSession *) { return make_ptr<ASTNode>(ASTNodeType::STATEMENT, 0); }

ASTNodePtr ast_create_identifier(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::ID, 0);
  ret->_is_named = true;
  return ret;
}

ASTNodePtr ast_create_identifier(CompilerSession *cs, const str &name) {
  auto ret = ast_create_identifier(cs);
  ret->set_data(name);
  return ret;
}

ASTNodePtr ast_create_parenthesis(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTNodeType::PARENTHESIS, ASTNode::OpPrecedence[ASTNodeType::PARENTHESIS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_func_call(CompilerSession *) {
  return make_ptr<ASTFunctionCall>();
}

} // namespace tanlang
