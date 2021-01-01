#include "src/ast/factory.h"
#include "src/common.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_control_flow.h"
#include "src/analysis/analysis.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "token.h"

namespace tanlang {

ASTNodePtr ast_create_return(CompilerSession *) {
  return make_ptr<ASTNode>(ASTType::RET, op_precedence[ASTType::RET]);
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

ASTNodePtr ast_create_string_literal(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::STRING_LITERAL, op_precedence[ASTType::STRING_LITERAL]);
  ret->_is_valued = true;
  ret->_is_typed = true;
  ret->_ty = create_ty(cs, Ty::STRING);
  ret->_ty->_is_lvalue = true;
  return ret;
}

ASTNodePtr ast_create_string_literal(CompilerSession *cs, const str &s) {
  auto ret = ast_create_string_literal(cs);
  ret->_value = s;
  return ret;
}

ASTNodePtr ast_create_array_literal(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ARRAY_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_numeric_literal(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::NUM_LITERAL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
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
  ret->_value = static_cast<uint64_t>(c);
  return ret;
}

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
  ret->_name = name;
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
  ret->_name = name;
  return ret;
}

ASTNodePtr ast_create_func_decl(CompilerSession *) {
  auto ret = make_ptr<ASTFunction>();
  ret->_is_named = true;
  return ret;
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

ASTNodePtr ast_create_arithmetic(CompilerSession *, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0);
  switch (hashed_string{op.c_str()}) {
    case "+"_hs:
      ret->_type = ASTType::SUM;
      break;
    case "-"_hs:
      ret->_type = ASTType::SUBTRACT;
      break;
    case "*"_hs:
      ret->_type = ASTType::MULTIPLY;
      break;
    case "/"_hs:
      ret->_type = ASTType::DIVIDE;
      break;
    case "%"_hs:
      ret->_type = ASTType::MOD;
      break;
    default:
      return nullptr;
  }
  ret->_lbp = op_precedence[ret->_type];
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_comparison(CompilerSession *, const str &op) {
  auto ret = make_ptr<ASTNode>(ASTType::INVALID, 0);
  switch (hashed_string{op.c_str()}) {
    case ">"_hs:
      ret->_type = ASTType::GT;
      break;
    case ">="_hs:
      ret->_type = ASTType::GE;
      break;
    case "<"_hs:
      ret->_type = ASTType::LT;
      break;
    case "<="_hs:
      ret->_type = ASTType::LE;
      break;
    case "=="_hs:
      ret->_type = ASTType::EQ;
      break;
    case "!="_hs:
      ret->_type = ASTType::NE;
      break;
    default:
      return nullptr;
  }
  ret->_lbp = op_precedence[ret->_type];
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_assignment(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ASSIGN, op_precedence[ASTType::ASSIGN]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_member_access(CompilerSession *cs) {
  auto ret = make_ptr<ASTMemberAccess>(ASTType::MEMBER_ACCESS, op_precedence[ASTType::MEMBER_ACCESS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_if(CompilerSession *) {
  auto ret = make_ptr<ASTIf>(ASTType::IF, op_precedence[ASTType::IF]);
  return ret;
}

ASTNodePtr ast_create_else(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::ELSE, op_precedence[ASTType::ELSE]);
  return ret;
}

ASTNodePtr ast_create_loop(CompilerSession *) {
  auto ret = make_ptr<ASTLoop>();
  return ret;
}

ASTNodePtr ast_create_break(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::BREAK, 0);
  return ret;
}

ASTNodePtr ast_create_continue(CompilerSession *cs) {
  auto ret = make_ptr<ASTNode>(ASTType::CONTINUE, 0);
  return ret;
}

ASTNodePtr ast_create_program(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::PROGRAM, 0);
  return ret;
}

ASTNodePtr ast_create_import(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::IMPORT, 0);
  ret->_is_named = true; /// name is the file imported
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
  ret->_name = name;
  return ret;
}

ASTNodePtr ast_create_parenthesis(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::PARENTHESIS, op_precedence[ASTType::PARENTHESIS]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTNodePtr ast_create_func_call(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::FUNC_CALL, 0);
  ret->_is_typed = true;
  ret->_is_valued = true;
  ret->_is_named = true;
  return ret;
}

ASTTyPtr ast_create_ty(CompilerSession *) {
  auto ret = make_ptr<ASTTy>();
  ret->_is_typed = true;
  ret->_is_valued = true; /// every type has its default value
  ret->_ty = ret;
  return ret;
}

ASTNodePtr ast_create_cast(CompilerSession *) {
  auto ret = make_ptr<ASTNode>(ASTType::CAST, op_precedence[ASTType::CAST]);
  ret->_is_typed = true;
  ret->_is_valued = true;
  return ret;
}

ASTTyPtr create_ty(CompilerSession *cs, Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue) {
  // TODO: cache
  auto ret = make_ptr<ASTTy>();
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  resolve_ty(cs, ret);
  return ret;
}

} // namespace tanlang
