#include "parser.h"
#include "src/ast/ast_ty.h"
#include <unordered_map>

namespace tanlang {

std::unordered_map<std::string, Ty> basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
     {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
     {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
     {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
     {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"i128", TY_OR(Ty::INT, Ty::BIT128)},
     {"u128", TY_OR3(Ty::INT, Ty::BIT128, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING},
     {"char", TY_OR(Ty::INT, Ty::BIT8)},};

std::unordered_map<std::string, Ty>
    qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

std::unordered_map<std::string, Ty> composite_tys = {{"struct", Ty::STRUCT}, {"array", Ty::ARRAY},};

void ASTTy::nud(Parser *parser) {
  Token *token = nullptr;
  while (parser->_curr_token < parser->_tokens.size()) {
    token = parser->get_curr_token();
    if (basic_tys.find(token->value) != basic_tys.end()) { // basic types
      _ty = TY_OR(_ty, basic_tys[token->value]);
    } else if (qualifier_tys.find(token->value) != qualifier_tys.end()) { // qualifiers
      if (TY_IS(_ty, Ty::POINTER) && token->value == "*") { // pointer to pointer (to ...)
        auto sub = std::make_shared<ASTTy>(token);
        // swap self and child
        sub->_ty = this->_ty;
        this->_ty = Ty::POINTER;
        _children.push_back(sub);
      } else { // other qualifiers other than pointer to pointers
        _ty = TY_OR(_ty, qualifier_tys[token->value]);
      }
    } else if (token->type == TokenType::ID) { // struct or array
      _type_name = token->value;
      if (composite_tys.find(token->value) == composite_tys.end()) {
        // TOOD: work with typedef
        _ty = TY_OR(_ty, Ty::STRUCT);
      } else {
        _ty = TY_OR(_ty, composite_tys[token->value]);
      }
    } else { break; }
    ++parser->_curr_token;
  }
}

} // namespace tanlang
