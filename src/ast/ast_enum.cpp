#include "src/ast/ast_enum.h"
#include "src/llvm_include.h"
#include "compiler_session.h"
#include "parser.h"
#include "token.h"

using namespace tanlang;

size_t ASTEnum::nud() {
  _end_index = _start_index + 1; /// skip "enum"
  auto name = _parser->parse<ASTType::ID>(_end_index, true);
  if (!name || name->_type != ASTType::ID) { error("Invalid enum name"); }
  _type_name = name->get_name();

  // TODO: parse optionally specified underlying type
  auto ty = ASTTy::Create(TY_OR(Ty::INT, Ty::BIT32));
  _default_value = ty->_default_value = static_cast<uint64_t>(0);
  _children.push_back(ty);

  /// enum body
  if (_parser->at(_end_index)->value != "{") { error("Invalid enum declaration"); }
  ++_end_index;
  uint64_t curr_enum_val = 0;
  while (!_parser->eof(_end_index) && _parser->at(_end_index)->value != "}") {
    // TODO: parse enum default value
    auto e = _parser->parse<ASTType::ID>(_end_index, true);
    _enum_values[e->get_name()] = curr_enum_val;
    ++curr_enum_val;
    if (_parser->at(_end_index)->value == ",") { ++_end_index; }
  }
  ++_end_index; /// skip '}'
  this->resolve();
  _cs->add(_type_name, this->shared_from_this());
  return _end_index;
}

uint64_t ASTEnum::get_enum_value(const str &value_name) const {
  auto search = _enum_values.find(value_name);
  if (search == _enum_values.end()) { return std::get<uint64_t>(_default_value); }
  else { return search->second; }
}

Value *ASTEnum::get_llvm_value(CompilerSession *cs) const {
  if (!_llvm_value) { _llvm_value = _children[0]->get_llvm_value(cs); }
  return _llvm_value;
}

Type *ASTEnum::to_llvm_type(CompilerSession *cs) const {
  if (!_llvm_type) { _llvm_type = _children[0]->to_llvm_type(cs); }
  return _llvm_type;
}

ASTEnum::ASTEnum(Token *t, size_t ti) : ASTTy(t, ti) {
  _type = ASTType::ENUM_DECL;
  _tyty = Ty::ENUM;
}

Value *ASTEnum::_codegen(CompilerSession *) { return nullptr; }
