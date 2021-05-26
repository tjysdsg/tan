#include "src/ast/factory.h"
#include "src/common.h"
#include "src/ast/ast_member_access.h"
#include "intrinsic.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

/// \section Declarations

ASTNodePtr ast_create_var_decl(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::VAR_DECL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  ret->_is_named = true;
  return ret;
}

ASTNodePtr ast_create_var_decl(CompilerSession *cs, const str &name, const ASTTyPtr &ty) {
  auto ret = ast_create_var_decl(cs);
  ret->_ty = make_ptr<ASTTy>(*ty);
  ret->_ty->_is_lvalue = true;
  ret->set_data(name);
  return ret;
}

ASTNodePtr ast_create_arg_decl(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ARG_DECL, 0);
  ret->_is_named = true;
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_arg_decl(CompilerSession *cs, const str &name, const ASTTyPtr &ty) {
  auto ret = ast_create_arg_decl(cs);
  ret->_ty = make_ptr<ASTTy>(*ty);
  ret->_ty->_is_lvalue = true;
  ret->set_data(name);
  return ret;
}

ASTNodePtr ast_create_func_decl(CompilerSession *) {
  return make_ptr<ASTFunction>();
}

ASTNodePtr ast_create_struct_decl(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::STRUCT_DECL, 0);
  ret->_is_named = true;
  return ret;
}

ASTNodePtr ast_create_enum_decl(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ENUM_DECL, 0);
  ret->_is_named = true;
  return ret;
}

/// \section Literals

ASTNodePtr ast_create_string_literal(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::STRING_LITERAL, ASTNode::OpPrecedence[ASTType::STRING_LITERAL]);
  ret->_is_valued = true;
  ret->_is_typed = true;
  ret->_ty = create_ty(cs, Ty::STRING);
  ret->_ty->_is_lvalue = true;
  return ret;
}

ASTNodePtr ast_create_string_literal(CompilerSession *cs, const str &s) {
  auto ret = ast_create_string_literal(cs);
  ret->set_data(s);
  return ret;
}

ASTNodePtr ast_create_array_literal(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ARRAY_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

static ASTNodePtr ast_create_numeric_literal() {
  auto ret = make_ptr<ASTNode>(ASTType::NUM_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_numeric_literal(CompilerSession *cs, uint64_t val, bool is_unsigned) {
  auto ret = ast_create_numeric_literal();
  // TODO: bit size
  ASTTyPtr ty = nullptr;
  if (is_unsigned) {
    ty = create_ty(cs, TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED));
  } else {
    ty = create_ty(cs, TY_OR(Ty::INT, Ty::BIT32));
  }
  ty->_default_value = val;
  ret->set_data(val);
  ret->_ty = ty;
  return ret;
}

ASTNodePtr ast_create_numeric_literal(CompilerSession *cs, double val) {
  auto ret = ast_create_numeric_literal();
  // TODO: float or double
  ASTTyPtr ty = create_ty(cs, Ty::FLOAT);
  ty->_default_value = val;
  ret->set_data(val);
  ret->_ty = ty;
  return ret;
}

ASTNodePtr ast_create_char_literal(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::CHAR_LITERAL, 0);
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
  auto ret = make_ptr<ASTNode>(ASTType::CAST, ASTNode::OpPrecedence[ASTType::CAST]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_arithmetic(CompilerSession *, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0);
  switch (hashed_string{op.c_str()}) {
    case "+"_hs:
      ret->set_node_type(ASTType::SUM);
      break;
    case "-"_hs:
      ret->set_node_type(ASTType::SUBTRACT);
      break;
    case "*"_hs:
      ret->set_node_type(ASTType::MULTIPLY);
      break;
    case "/"_hs:
      ret->set_node_type(ASTType::DIVIDE);
      break;
    case "%"_hs:
      ret->set_node_type(ASTType::MOD);
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
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0);
  switch (hashed_string{op.c_str()}) {
    case ">"_hs:
      ret->set_node_type(ASTType::GT);
      break;
    case ">="_hs:
      ret->set_node_type(ASTType::GE);
      break;
    case "<"_hs:
      ret->set_node_type(ASTType::LT);
      break;
    case "<="_hs:
      ret->set_node_type(ASTType::LE);
      break;
    case "=="_hs:
      ret->set_node_type(ASTType::EQ);
      break;
    case "!="_hs:
      ret->set_node_type(ASTType::NE);
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
  auto ret = make_ptr<ASTNode>(ASTType::ASSIGN, ASTNode::OpPrecedence[ASTType::ASSIGN]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_member_access(CompilerSession *) {
  auto ret = make_ptr<ASTMemberAccess>(ASTType::MEMBER_ACCESS, ASTNode::OpPrecedence[ASTType::MEMBER_ACCESS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_return(CompilerSession *) {
  return make_ptr<ASTNode>(ASTType::RET, ASTNode::OpPrecedence[ASTType::RET]);
}

ASTNodePtr ast_create_not(CompilerSession *) {
  /// logical not or bitwise not
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0);
  ret->_is_valued = true;
  ret->_is_typed = true;
  return ret;
}

ASTNodePtr ast_create_ampersand(CompilerSession *) {
  /// address_of or binary and
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0);
  ret->_is_valued = true;
  ret->_is_typed = true;
  return ret;
}

ASTNodePtr ast_create_address_of(CompilerSession *, ParsableASTNodePtr p) {
  auto ret = make_ptr<ASTNode>(ASTType::ADDRESS_OF, ASTNode::OpPrecedence[ASTType::ADDRESS_OF]);
  ret->_is_valued = true;
  ret->_is_typed = true;
  ret->append_child(p);
  return ret;
}

/// \section Control flow

ASTNodePtr ast_create_if(CompilerSession *) {
  auto ret = make_ptr<ASTIf>(ASTType::IF, ASTNode::OpPrecedence[ASTType::IF]);
  return ret;
}

ASTNodePtr ast_create_else(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ELSE, ASTNode::OpPrecedence[ASTType::ELSE]);
  return ret;
}

ASTNodePtr ast_create_loop(CompilerSession *) {
  auto ret = make_ptr<ASTLoop>();
  return ret;
}

ASTNodePtr ast_create_break(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::BREAK, 0);
  return ret;
}

ASTNodePtr ast_create_continue(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::CONTINUE, 0);
  return ret;
}

/// \section Others

ASTNodePtr ast_create_program(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::PROGRAM, 0);
  return ret;
}

ASTNodePtr ast_create_import(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::IMPORT, 0);
  ret->_is_named = true; /// name is the file imported
  return ret;
}

ASTNodePtr ast_create_intrinsic(CompilerSession *) {
  auto ret = make_ptr<Intrinsic>();
  return ret;
}

ASTNodePtr ast_create_statement(CompilerSession *) { return make_ptr<ASTNode>(ASTType::STATEMENT, 0); }

ASTNodePtr ast_create_identifier(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ID, 0);
  ret->_is_named = true;
  return ret;
}

ASTNodePtr ast_create_identifier(CompilerSession *cs, const str &name) {
  auto ret = ast_create_identifier(cs);
  ret->set_data(name);
  return ret;
}

ASTNodePtr ast_create_parenthesis(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::PARENTHESIS, ASTNode::OpPrecedence[ASTType::PARENTHESIS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_func_call(CompilerSession *) {
  return make_ptr<ASTFunctionCall>();
}

ASTTyPtr ast_create_ty(CompilerSession *) {
  return make_ptr<ASTTy>();
}

ASTTyPtr create_ty(CompilerSession *cs, Ty t, vector<ASTTyPtr> sub_tys, bool is_lvalue) {
  // TODO: cache
  auto ret = make_ptr<ASTTy>();
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->get_children().insert(ret->get_children().begin(), sub_tys.begin(), sub_tys.end());
  TypeSystem::ResolveTy(cs, ret);
  return ret;
}

} // namespace tanlang
