#include "src/ast/ast_ty.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_number_literal.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/common.h"

using namespace tanlang;

std::unordered_map<std::string, Ty> basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
        {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
        {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
        {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
        {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING}, {"char", Ty::CHAR},
        {"bool", Ty::BOOL},};

std::unordered_map<std::string, Ty>
    qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

ASTTyPtr ASTTy::find_cache(Ty t, std::vector<ASTNodePtr> sub_tys, bool is_lvalue) {
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

std::shared_ptr<ASTTy> ASTTy::Create(Ty t, std::vector<ASTNodePtr> sub_tys, bool is_lvalue) {
  auto ret = ASTTy::find_cache(t, sub_tys, is_lvalue);
  if (ret) { return ret; }
  ret = std::make_shared<ASTTy>(nullptr, 0);
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  ret->resolve();
  return ret;
}

Value *ASTTy::get_llvm_value(CompilerSession *cs) const {
  TAN_ASSERT(_resolved);
  Ty base = TY_GET_BASE(_tyty);
  Value *ret = nullptr;
  Type *type = this->to_llvm_type(cs);
  switch (base) {
    case Ty::INT:
    case Ty::CHAR:
    case Ty::BOOL:
      ret = ConstantInt::get(type, std::get<uint64_t>(_default_value));
      break;
    case Ty::FLOAT:
      ret = ConstantFP::get(type, std::get<float>(_default_value));
      break;
    case Ty::DOUBLE:
      ret = ConstantFP::get(type, std::get<double>(_default_value));
      break;
    case Ty::STRING:
      ret = cs->get_builder()->CreateGlobalStringPtr(std::get<std::string>(_default_value));
      break;
    case Ty::VOID:
      TAN_ASSERT(false);
      break;
    case Ty::STRUCT: {
      auto st = ast_cast<ASTStruct>(cs->get(_type_name));
      TAN_ASSERT(st);
      ret = st->get_llvm_value(cs);
      break;
    }
    case Ty::POINTER:
      ret = ConstantPointerNull::get((PointerType *) type);
      break;
    case Ty::ARRAY: {
      auto *e_type = _children[0]->to_llvm_type(cs);
      ret = create_block_alloca(cs->get_builder()->GetInsertBlock(), e_type, _n_elements, "const_array");
      for (size_t i = 0; i < _n_elements; ++i) {
        auto *idx = cs->get_builder()->getInt32((unsigned) i);
        auto *e_val = _children[i]->get_llvm_value(cs);
        auto *e_ptr = cs->get_builder()->CreateGEP(ret, idx);
        cs->get_builder()->CreateStore(e_val, e_ptr);
      }
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

Type *ASTTy::to_llvm_type(CompilerSession *cs) const {
  TAN_ASSERT(_resolved);
  Ty base = TY_GET_BASE(_tyty);
  llvm::Type *type = nullptr;
  switch (base) {
    case Ty::INT:
      type = cs->get_builder()->getIntNTy((unsigned) _size_bits);
      break;
    case Ty::CHAR:
      type = cs->get_builder()->getInt8Ty();
      break;
    case Ty::BOOL:
      type = cs->get_builder()->getInt1Ty();
      break;
    case Ty::FLOAT:
      type = cs->get_builder()->getFloatTy();
      break;
    case Ty::DOUBLE:
      type = cs->get_builder()->getDoubleTy();
      break;
    case Ty::STRING:
      type = cs->get_builder()->getInt8PtrTy(); /// str as char*
      break;
    case Ty::VOID:
      type = cs->get_builder()->getVoidTy();
      break;
    case Ty::STRUCT: {
      /// ASTStruct must override this, otherwise ASTStruct must override this, otherwise ASTStruct must override ...
      auto st = ast_cast<ASTStruct>(cs->get(_type_name));
      type = st->to_llvm_type(cs);
      break;
    }
    case Ty::ARRAY: /// during analysis phase, array is different from pointer, but during codegen, they are the same
    case Ty::POINTER: {
      auto e_type = ast_cast<ASTTy>(_children[0])->to_llvm_type(cs);
      type = e_type->getPointerTo();
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return type;
}

Metadata *ASTTy::to_llvm_meta(CompilerSession *cs) const {
  TAN_ASSERT(_resolved);
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
      ret = cs->get_di_builder()->createBasicType(_type_name, _size_bits, _dwarf_encoding);
      break;
    case Ty::STRING: {
      auto *e_di_type = cs->get_di_builder()->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = cs->get_di_builder()
          ->createPointerType(e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    case Ty::STRUCT: {
      DIFile *di_file = cs->get_di_file();
      auto st = cs->get(_type_name);
      size_t n = st->_children.size();
      std::vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        auto e = st->_children[i]; // ASTVarDecl
        elements.push_back(e->get_ty()->to_llvm_meta(cs));
      }
      ret = cs->get_di_builder()
          ->createStructType(cs->get_current_di_scope(),
              _type_name,
              di_file,
              (unsigned) st->_token->l,
              _size_bits,
              (unsigned) _align_bits,
              DINode::DIFlags::FlagZero,
              nullptr,
              cs->get_di_builder()->getOrCreateArray(elements),
              0,
              nullptr,
              _type_name);
      break;
    }
    case Ty::ARRAY:
    case Ty::POINTER: {
      auto e = ast_cast<ASTTy>(_children[0]);
      auto *e_di_type = e->to_llvm_meta(cs);
      ret = cs->get_di_builder()
          ->createPointerType((DIType *) e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

bool ASTTy::operator==(const ASTTy &other) const {
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

void ASTTy::resolve() {
  if (_resolved) { return; }
  _ty = this->shared_from_this();
  /// resolve children if they are ASTTy
  for (auto c: _children) {
    auto t = ast_cast<ASTTy>(c);
    if (t && t->_type == ASTType::TY && !t->_resolved) { t->resolve(); }
  }
  auto *tm = Compiler::GetDefaultTargetMachine(); /// can't use _cs here, cuz some ty are created by ASTTy::Create()
  Ty base = TY_GET_BASE(_tyty);
  Ty qual = TY_GET_QUALIFIER(_tyty);
  switch (base) {
    case Ty::INT: {
      _size_bits = 32;
      _type_name = "i32";
      _is_int = true;
      _default_value.emplace<uint64_t>(0);
      if (TY_IS(qual, Ty::BIT8)) {
        _size_bits = 8;
        _type_name = "i8";
      } else if (TY_IS(qual, Ty::BIT16)) {
        _size_bits = 16;
        _type_name = "i16";
      } else if (TY_IS(qual, Ty::BIT64)) {
        _size_bits = 64;
        _type_name = "i64";
      }
      if (TY_IS(qual, Ty::UNSIGNED)) {
        _is_unsigned = true;
        if (_size_bits == 8) {
          _dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
        } else {
          _dwarf_encoding = llvm::dwarf::DW_ATE_unsigned;
        }
      } else {
        if (_size_bits == 8) {
          _dwarf_encoding = llvm::dwarf::DW_ATE_signed_char;
        } else {
          _dwarf_encoding = llvm::dwarf::DW_ATE_signed;
        }
      }
      break;
    }
    case Ty::CHAR:
      _type_name = "char";
      _size_bits = 8;
      _dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
      _is_unsigned = true;
      _default_value.emplace<uint64_t>(0);
      _is_int = true;
      break;
    case Ty::BOOL:
      _type_name = "bool";
      _size_bits = 1;
      _dwarf_encoding = llvm::dwarf::DW_ATE_boolean;
      _default_value.emplace<uint64_t>(0);
      _is_bool = true;
      break;
    case Ty::FLOAT:
      _type_name = "float";
      _size_bits = 32;
      _dwarf_encoding = llvm::dwarf::DW_ATE_float;
      _default_value.emplace<float>(0);
      _is_float = true;
      break;
    case Ty::DOUBLE:
      _type_name = "double";
      _size_bits = 64;
      _dwarf_encoding = llvm::dwarf::DW_ATE_float;
      _default_value.emplace<double>(0);
      _is_double = true;
      break;
    case Ty::STRING:
      _type_name = "u8*";
      _size_bits = tm->getPointerSizeInBits(0);
      _default_value.emplace<std::string>("");
      _align_bits = 8;
      _is_ptr = true;
      break;
    case Ty::VOID:
      _type_name = "void";
      _size_bits = 0;
      _dwarf_encoding = llvm::dwarf::DW_ATE_signed;
      break;
    case Ty::STRUCT: {
      /// align size is the max element size, if no element, 8 bits
      /// size is the number of elements * align size
      auto st = ast_cast<ASTStruct>(_cs->get(_type_name));
      _align_bits = 8;
      size_t n = st->get_n_elements();
      for (size_t i = 0; i < n; ++i) {
        auto et = ast_cast<ASTTy>(st->_children[i]);
        auto s = et->get_size_bits();
        if (s > _align_bits) { s = _align_bits; }
      }
      _size_bits = n * _align_bits;
      _is_struct = true;
      break;
    }
    case Ty::ARRAY: {
      auto et = ast_cast<ASTTy>(_children[0]);
      auto s = ast_cast<ASTNumberLiteral>(_children[1]);
      TAN_ASSERT(et);
      TAN_ASSERT(s);
      _n_elements = _children.size();
      _type_name = "[" + et->get_type_name() + ", " + std::to_string(_n_elements) + "]";
      _is_ptr = true;
      _is_array = true;
      _size_bits = tm->getPointerSizeInBits(0);
      _align_bits = et->get_size_bits();
      _dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    case Ty::POINTER: {
      auto e = ast_cast<ASTTy>(_children[0]);
      TAN_ASSERT(e);
      e->resolve();
      _type_name = e->get_type_name() + "*";
      _size_bits = tm->getPointerSizeInBits(0);
      _align_bits = e->get_size_bits();
      _is_ptr = true;
      _dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  _resolved = true;
}

/// current token should be "[" when this is called
/// this will set _type_name
size_t ASTTy::nud_array() {
  _end_index = _start_index + 1; /// skip "["
  ASTNodePtr element = nullptr;
  /// element type
  if (_parser->at(_end_index)->value == "]") { /// empty
    report_code_error(_parser->at(_end_index), "The array type and size must be specified");
  } else {
    element = std::make_shared<ASTTy>(_parser->at(_end_index), _end_index);
    _end_index = element->parse(_parser, _cs); /// this set the _type_name of child
  }
  _parser->peek(_end_index, TokenType::PUNCTUATION, ",");
  ++_end_index; /// skip ","

  /// size
  auto size = _parser->peek(_end_index);
  if (size->_type != ASTType::NUM_LITERAL) {
    report_code_error(_parser->at(_end_index), "Expect an unsigned integer");
  }
  auto size1 = ast_cast<ASTNumberLiteral>(size);
  if (size1->is_float() || size1->_ivalue < 0) {
    report_code_error(_parser->at(_end_index), "Expect an unsigned integer");
  }
  _n_elements = static_cast<size_t>(size1->_ivalue);
  for (size_t i = 0; i < _n_elements; ++i) { _children.push_back(element->get_ty()); }
  /// set _type_name to [<element type>, <n_elements>]
  _type_name = "[" + _type_name + ", " + std::to_string(_n_elements) + "]";
  ++_end_index; /// skip "]"
  return _end_index;
}

size_t ASTTy::nud() {
  _end_index = _start_index;
  Token *token = nullptr;
  while (!_parser->eof(_end_index)) {
    token = _parser->at(_end_index);
    if (basic_tys.find(token->value) != basic_tys.end()) { /// base types
      _tyty = TY_OR(_tyty, basic_tys[token->value]);
    } else if (qualifier_tys.find(token->value) != qualifier_tys.end()) { /// TODO: qualifiers
      if (token->value == "*") { /// pointer
        auto sub = std::make_shared<ASTTy>(token, _end_index + 1);
        /// swap self and child
        sub->_tyty = this->_tyty;
        _tyty = Ty::POINTER;
        _children.push_back(sub);
        auto ei = sub->parse(_parser, _cs);
        if (_end_index + 1 != ei) { _end_index = ei - 1; }
      }
    } else if (token->type == TokenType::ID) { /// struct or array
      // TODO: identify type aliases
      _type_name = token->value; /// _type_name is the name of the struct
      _tyty = TY_OR(_tyty, Ty::STRUCT);
    } else if (token->value == "[") {
      _tyty = TY_OR(_tyty, Ty::ARRAY);
      _end_index = nud_array(); /// set _type_name in nud_array()
    } else { break; }
    ++_end_index;
  }
  resolve(); /// fill in relevant member variables
  return _end_index;
}

ASTTyPtr ASTTy::get_ptr_to() const { return ASTTy::Create(Ty::POINTER, {get_ty()}, false); }

bool ASTTy::is_array() const {
  TAN_ASSERT(_resolved);
  return _is_array;
}

bool ASTTy::is_ptr() const {
  TAN_ASSERT(_resolved);
  return _is_ptr;
}

bool ASTTy::is_float() const {
  TAN_ASSERT(_resolved);
  return _is_float;
}

bool ASTTy::is_double() const {
  TAN_ASSERT(_resolved);
  return _is_double;
}

bool ASTTy::is_int() const {
  TAN_ASSERT(_resolved);
  return _is_int;
}

bool ASTTy::is_bool() const {
  TAN_ASSERT(_resolved);
  return _is_bool;
}

bool ASTTy::is_unsigned() const {
  TAN_ASSERT(_resolved);
  return _is_unsigned;
}

bool ASTTy::is_struct() const {
  TAN_ASSERT(_resolved);
  return _is_struct;
}

bool ASTTy::is_floating() const {
  TAN_ASSERT(_resolved);
  return _is_float || _is_double;
}

bool ASTTy::is_lvalue() const {
  TAN_ASSERT(_resolved);
  return _is_lvalue;
}

bool ASTTy::is_typed() const { return true; }

size_t ASTTy::get_size_bits() const {
  TAN_ASSERT(_resolved);
  return _size_bits;
}

std::string ASTTy::get_type_name() const {
  TAN_ASSERT(!_type_name.empty());
  return _type_name;
}

std::string ASTTy::to_string(bool print_prefix) const {
  return ASTNode::to_string(print_prefix) + " " + get_type_name();
}

ASTTy::ASTTy(Token *token, size_t token_index) : ASTNode(ASTType::TY, 0, 0, token, token_index) {}

void ASTTy::set_is_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }

bool ASTTy::operator!=(const ASTTy &other) const { return !this->operator==(other); }

ASTTyPtr ASTTy::get_contained_ty() const {
  if (_tyty == Ty::STRING) { return ASTTy::Create(Ty::CHAR, std::vector<ASTNodePtr>(), false); }
  else if (_is_ptr) {
    TAN_ASSERT(_children.size());
    auto ret = ast_cast<ASTTy>(_children[0]);
    TAN_ASSERT(ret);
    return ret;
  } else { return nullptr; }
}

size_t ASTTy::get_n_elements() const { return _n_elements; }
