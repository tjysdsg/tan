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
        {"char", TY_OR(Ty::INT, Ty::BIT8)}, {"bool", Ty::BOOL},};

std::unordered_map<std::string, Ty>
    qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

/// current token should be "[" when this is called
/// this will set _type_name
size_t ASTTy::nud_array(Parser *parser) {
  _end_index = _start_index + 1; /// skip "["
  /// element type
  if (parser->at(_end_index)->value == "]") { /// empty
    report_code_error(parser->at(_end_index), "The array type and size must be specified");
  } else {
    auto child = std::make_shared<ASTTy>(parser->at(_end_index), _end_index);
    _end_index = child->parse(parser); /// this set the _type_name of child
    _children.push_back(child);
    _type_name = child->_type_name;
  }
  parser->peek(_end_index, TokenType::PUNCTUATION, ",");
  ++_end_index; /// skip ","

  /// size
  auto size = parser->peek(_end_index);
  if (size->_type != ASTType::NUM_LITERAL) {
    report_code_error(parser->at(_end_index), "Expect an unsigned integer");
  }
  auto size1 = ast_cast<ASTNumberLiteral>(size);
  if (size1->is_float() || size1->_ivalue < 0) {
    report_code_error(parser->at(_end_index), "Expect an unsigned integer");
  }
  _n_elements = static_cast<size_t>(size1->_ivalue);
  /// set _type_name to [<element type>, <n_elements>]
  _type_name = "[" + _type_name + ", " + std::to_string(_n_elements) + "]";
  ++_end_index; /// skip "]"
  return _end_index;
}

size_t ASTTy::nud(Parser *parser) {
  _end_index = _start_index;
  Token *token = nullptr;
  while (!parser->eof(_end_index)) {
    token = parser->at(_end_index);
    if (basic_tys.find(token->value) != basic_tys.end()) { /// base types
      _ty = TY_OR(_ty, basic_tys[token->value]);
      _type_name += token->value; /// just append the type name for basic types and qualifiers
    } else if (qualifier_tys.find(token->value) != qualifier_tys.end()) { /// qualifiers
      if (TY_IS(_ty, Ty::POINTER) && token->value == "*") { /// pointer to pointer (to ...)
        auto sub = std::make_shared<ASTTy>(token, _end_index);
        /// swap self and child, so this is a pointer with no basic type, and the child is a pointer to something
        sub->_ty = this->_ty;
        this->_ty = Ty::POINTER;
        /// remember to set the name of sub
        sub->_type_name = _type_name;
        _children.push_back(sub);
      } else { /// qualifiers other than pointer to pointers
        _ty = TY_OR(_ty, qualifier_tys[token->value]);
      }
      _type_name += token->value; /// just append the type name for basic types and qualifiers
    } else if (token->type == TokenType::ID) { /// struct or array
      // TODO: identify type aliases
      _type_name = token->value; /// _type_name is the name of the struct
      _ty = TY_OR(_ty, Ty::STRUCT);
    } else if (token->value == "[") {
      _ty = TY_OR(_ty, Ty::ARRAY);
      _end_index = nud_array(parser); /// set _type_name in nud_array()
    } else { break; }
    ++_end_index;
  }
  resolve(); /// fill in relevant member variables
  return _end_index;
}

} // namespace tanlang
