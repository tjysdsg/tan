#include "src/ast/ast_ty.h"
#include "src/analysis/analysis.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/common.h"

using namespace tanlang;

umap<str, Ty> basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
        {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
        {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
        {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
        {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING}, {"char", Ty::CHAR},
        {"bool", Ty::BOOL},};

umap<str, Ty> qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

ASTTyPtr ASTTy::find_cache(Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue) {
  auto find = ASTTy::_cache.find(t);
  if (find == ASTTy::_cache.end()) { return nullptr; }
  if (find->second->_is_lvalue != is_lvalue) { return nullptr; }
  auto ret = find->second;

  if (sub_tys.size() != ret->_children.size()) { return nullptr; }
  size_t idx = 0;
  for (const auto &sub : sub_tys) {
    auto t1 = ast_cast<ASTTy>(sub);
    auto t2 = ast_cast<ASTTy>(ret->_children[idx]);
    if (t1->_tyty != t2->_tyty) { return nullptr; }
    if (t1->_is_lvalue != t2->_is_lvalue) { return nullptr; }
    ++idx;
  }
  return ret;
}

Metadata *ASTTy::to_llvm_meta(CompilerSession *cs) {
  resolve(this->ptr_from_this());
  Ty base = TY_GET_BASE(_tyty);
  // TODO: Ty qual = TY_GET_QUALIFIER(_tyty);
  DIType *ret = nullptr;
  switch (base) {
    case Ty::CHAR:
    case Ty::INT:
    case Ty::BOOL:
    case Ty::FLOAT:
    case Ty::VOID:
    case Ty::DOUBLE:
    case Ty::ENUM:
      ret = cs->_di_builder->createBasicType(_type_name, _size_bits, _dwarf_encoding);
      break;
    case Ty::STRING: {
      auto *e_di_type = cs->_di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = cs->_di_builder->createPointerType(e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    case Ty::STRUCT: {
      DIFile *di_file = cs->get_di_file();
      size_t n = _children.size();
      vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        auto e = _children[i]; // ASTVarDecl
        elements.push_back(get_ty(e)->to_llvm_meta(cs));
      }
      ret = cs->_di_builder
          ->createStructType(cs->get_current_di_scope(),
              _type_name,
              di_file,
              (unsigned) _token->l,
              _size_bits,
              (unsigned) _align_bits,
              DINode::DIFlags::FlagZero,
              nullptr,
              cs->_di_builder->getOrCreateArray(elements),
              0,
              nullptr,
              _type_name);
      break;
    }
    case Ty::ARRAY:
    case Ty::POINTER: {
      auto e = ast_cast<ASTTy>(_children[0]);
      auto *e_di_type = e->to_llvm_meta(cs);
      ret = cs->_di_builder
          ->createPointerType((DIType *) e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

bool ASTTy::operator==(const ASTTy &other) {
  #define CHECK(val) if (this->val != other.val) { return false; }
  CHECK(_size_bits)
  CHECK(_align_bits)
  CHECK(_is_ptr)
  CHECK(_is_float)
  CHECK(_is_double)
  CHECK(_is_int)
  CHECK(_is_unsigned)
  CHECK(_is_struct)
  CHECK(_is_bool)
  #undef CHECK

  if (!_children.empty()) {
    size_t n = _children.size();
    if (n != other._children.size()) { return false; }
    for (size_t i = 0; i < n; ++i) {
      return ast_cast<ASTTy>(_children[i])->operator==(*ast_cast<ASTTy>(other._children[i]));
    }
  }
  return true;
}

/// current token should be "[" when this is called
/// this will set _type_name
size_t ASTTy::nud_array() {
  _end_index = _start_index + 1; /// skip "["
  ASTNodePtr element = nullptr;
  /// element type
  if (_parser->at(_end_index)->value == "]") { /// empty
    error("The array type and size must be specified");
  } else {
    element = std::make_shared<ASTTy>(_parser->at(_end_index), _end_index);
    _end_index = element->parse(_parser, _cs); /// this set the _type_name of child
  }
  _parser->peek(_end_index, TokenType::PUNCTUATION, ",");
  ++_end_index; /// skip ","

  /// size
  auto size = _parser->peek(_end_index);
  if (size->_type != ASTType::NUM_LITERAL) { error(_end_index, "Expect an unsigned integer"); }
  auto size1 = ast_cast<ASTNumberLiteral>(size);
  if (size1->is_float() || size1->_ivalue < 0) { error(_end_index, "Expect an unsigned integer"); }
  _n_elements = static_cast<size_t>(size1->_ivalue);
  for (size_t i = 0; i < _n_elements; ++i) { _children.push_back(get_ty(element)); }
  /// set _type_name to [<element type>, <n_elements>]
  _type_name = "[" + _type_name + ", " + std::to_string(_n_elements) + "]";
  ++_end_index; /// skip "]"
  return _end_index;
}

size_t ASTTy::nud_struct() {
  _end_index = _start_index + 1; /// skip "struct"
  /// struct typename
  auto id = _parser->parse<ASTType::ID>(_end_index, true);
  _type_name = get_name(id);

  auto forward_decl = _cs->get(_type_name);
  if (!forward_decl) {
    _cs->add(_type_name, this->ptr_from_this()); /// add self to current scope
  } else {
    /// replace forward decl with self (even if this is a forward declaration too)
    _cs->set(_type_name, this->ptr_from_this());
  }

  /// struct body
  if (_parser->at(_end_index)->value == "{") {
    auto comp_stmt = _parser->next_expression(_end_index);
    if (!comp_stmt || comp_stmt->_type != ASTType::STATEMENT) { error(_end_index, "Invalid struct body"); }

    /// resolve_ty member names and types
    auto members = comp_stmt->_children;
    ASTNodePtr var_decl = nullptr;
    size_t n = comp_stmt->_children.size();
    _member_names.reserve(n);
    _children.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      if (members[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
        var_decl = members[i];
        _children.push_back(get_ty(var_decl));
      } else if (members[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
        var_decl = members[i]->_children[0];
        auto initial_value = members[i]->_children[1];
        // TODO: check if value is compile-time known
        _children.push_back(get_ty(initial_value)); /// initial value is set to ASTTy in ASTLiteral::get_ty()
      } else { members[i]->error("Invalid struct member"); }
      auto name = get_name(var_decl);
      _member_names.push_back(name);
      _member_indices[name] = i;
    }
    resolve(this->ptr_from_this());
  } else { _is_forward_decl = true; }
  return _end_index;
}

size_t ASTTy::nud() {
  _end_index = _start_index;
  Token *token;
  while (!_parser->eof(_end_index)) {
    token = _parser->at(_end_index);
    if (basic_tys.find(token->value) != basic_tys.end()) { /// base types
      _tyty = TY_OR(_tyty, basic_tys[token->value]);
    } else if (qualifier_tys.find(token->value) != qualifier_tys.end()) { /// TODO: qualifiers
      if (token->value == "*") { /// pointer
        /// swap self and child
        auto sub = std::make_shared<ASTTy>(*this);
        _tyty = Ty::POINTER;
        _children.clear(); /// clear but memory stays
        _children.push_back(sub);
      }
    } else if (token->type == TokenType::ID) { /// struct or enum
      // TODO: identify type aliases
      _type_name = token->value;
      auto ty = ast_cast<ASTTy>(_cs->get(_type_name));
      if (ty) { *this = *ty; }
      else { error("Invalid type name"); }
    } else if (token->value == "[") {
      _tyty = Ty::ARRAY;
      _end_index = nud_array(); /// set _type_name in nud_array()
      break;
    } else if (token->value == "struct") {
      _tyty = Ty::STRUCT;
      _end_index = nud_struct();
      break;
    } else { break; }
    ++_end_index;
  }
  resolve(ptr_from_this());
  return _end_index;
}

str ASTTy::to_string(bool print_prefix) { return ASTNode::to_string(print_prefix) + " " + get_type_name(); }

bool ASTTy::operator!=(const ASTTy &other) { return !this->operator==(other); }

ASTTy &ASTTy::operator=(const ASTTy &other) {
  _tyty = other._tyty;
  _default_value = other._default_value;
  _type_name = other._type_name;
  _children = other._children;
  _size_bits = other._size_bits;
  _align_bits = other._align_bits;
  _dwarf_encoding = other._dwarf_encoding;
  _is_ptr = other._is_ptr;
  _is_float = other._is_float;
  _is_array = other._is_array;
  _is_int = other._is_int;
  _is_unsigned = other._is_unsigned;
  _is_struct = other._is_struct;
  _is_bool = other._is_bool;
  _is_enum = other._is_enum;
  _is_lvalue = other._is_lvalue;
  _is_forward_decl = other._is_forward_decl;
  return const_cast<ASTTy &>(*this);
}

ASTTy &ASTTy::operator=(ASTTy &&other) {
  _tyty = other._tyty;
  _default_value = other._default_value;
  _type_name = other._type_name;
  _children = other._children;
  _size_bits = other._size_bits;
  _align_bits = other._align_bits;
  _dwarf_encoding = other._dwarf_encoding;
  _is_ptr = other._is_ptr;
  _is_float = other._is_float;
  _is_array = other._is_array;
  _is_int = other._is_int;
  _is_unsigned = other._is_unsigned;
  _is_struct = other._is_struct;
  _is_bool = other._is_bool;
  _is_enum = other._is_enum;
  _is_lvalue = other._is_lvalue;
  _is_forward_decl = other._is_forward_decl;
  return const_cast<ASTTy &>(*this);
}

ASTTy::ASTTy() : ASTNode(ASTType::TY, 0, 0) {}
