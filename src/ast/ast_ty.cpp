#include "src/ast/ast_ty.h"
#include "src/ast/ast_number_literal.h"
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

std::shared_ptr<ASTTy> ASTTy::Create(Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue) {
  auto ret = ASTTy::find_cache(t, sub_tys, is_lvalue);
  if (ret) { return ret; }
  ret = std::make_shared<ASTTy>(nullptr, 0);
  ret->_tyty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  ret->resolve();
  return ret;
}

Value *ASTTy::get_llvm_value(CompilerSession *cs) {
  auto *builder = cs->_builder;
  resolve();
  Ty base = TY_GET_BASE(_tyty);
  Value *ret = nullptr;
  Type *type = this->to_llvm_type(cs);
  switch (base) {
    case Ty::INT:
    case Ty::CHAR:
    case Ty::BOOL:
    case Ty::ENUM:
      ret = ConstantInt::get(type, std::get<uint64_t>(_default_value));
      break;
    case Ty::FLOAT:
      ret = ConstantFP::get(type, std::get<float>(_default_value));
      break;
    case Ty::DOUBLE:
      ret = ConstantFP::get(type, std::get<double>(_default_value));
      break;
    case Ty::STRING:
      ret = builder->CreateGlobalStringPtr(std::get<str>(_default_value));
      break;
    case Ty::VOID:
      TAN_ASSERT(false);
    case Ty::STRUCT: {
      vector<llvm::Constant *> values{};
      size_t n = _children.size();
      for (size_t i = 1; i < n; ++i) {
        values.push_back((llvm::Constant *) _children[i]->get_ty()->get_llvm_value(cs));
      }
      ret = ConstantStruct::get((StructType *) to_llvm_type(cs), values);
      break;
    }
    case Ty::POINTER:
      ret = ConstantPointerNull::get((PointerType *) type);
      break;
    case Ty::ARRAY: {
      auto *e_type = _children[0]->to_llvm_type(cs);
      ret = create_block_alloca(builder->GetInsertBlock(), e_type, _n_elements, "const_array");
      for (size_t i = 0; i < _n_elements; ++i) {
        auto *idx = builder->getInt32((unsigned) i);
        auto *e_val = _children[i]->get_llvm_value(cs);
        auto *e_ptr = builder->CreateGEP(ret, idx);
        builder->CreateStore(e_val, e_ptr);
      }
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

Type *ASTTy::to_llvm_type(CompilerSession *cs) {
  auto *builder = cs->_builder;
  resolve();
  Ty base = TY_GET_BASE(_tyty);
  llvm::Type *type = nullptr;
  switch (base) {
    case Ty::INT:
      type = builder->getIntNTy((unsigned) _size_bits);
      break;
    case Ty::CHAR:
      type = builder->getInt8Ty();
      break;
    case Ty::BOOL:
      type = builder->getInt1Ty();
      break;
    case Ty::FLOAT:
      type = builder->getFloatTy();
      break;
    case Ty::DOUBLE:
      type = builder->getDoubleTy();
      break;
    case Ty::STRING:
      type = builder->getInt8PtrTy(); /// str as char*
      break;
    case Ty::VOID:
      type = builder->getVoidTy();
      break;
    case Ty::ENUM:
      type = _children[0]->to_llvm_type(cs);
      break;
    case Ty::STRUCT: {
      auto *struct_type = StructType::create(*cs->get_context(), _type_name);
      vector<Type *> body{};
      size_t n = _children.size();
      body.reserve(n);
      for (size_t i = 1; i < n; ++i) { body.push_back(_children[i]->to_llvm_type(cs)); }
      struct_type->setBody(body);
      type = struct_type;
      break;
    }
    case Ty::ARRAY: /// during analysis phase, array is different from pointer, but during _codegen, they are the same
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

Metadata *ASTTy::to_llvm_meta(CompilerSession *cs) {
  resolve();
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
        elements.push_back(e->get_ty()->to_llvm_meta(cs));
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

void ASTTy::resolve() {
  Ty base = TY_GET_BASE(_tyty);
  Ty qual = TY_GET_QUALIFIER(_tyty);
  if (_resolved) {
    if (base == Ty::STRUCT) {
      if (!_is_forward_decl) { return; }
    } else { return; }
  }
  _ty = this->shared_from_this();
  /// resolve children if they are ASTTy
  for (auto c: _children) {
    auto t = ast_cast<ASTTy>(c);
    if (t && t->_type == ASTType::TY && !t->_resolved) { t->resolve(); }
  }
  auto *tm = Compiler::GetDefaultTargetMachine(); /// can't use _cs here, cuz some ty are created by ASTTy::Create()
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
      _default_value.emplace<str>("");
      _align_bits = 8;
      _is_ptr = true;
      break;
    case Ty::VOID:
      _type_name = "void";
      _size_bits = 0;
      _dwarf_encoding = llvm::dwarf::DW_ATE_signed;
      break;
    case Ty::ENUM: {
      auto sub = ast_cast<ASTTy>(_children[0]);
      TAN_ASSERT(sub);
      _size_bits = sub->_size_bits;
      _align_bits = sub->_align_bits;
      _dwarf_encoding = sub->_dwarf_encoding;
      _default_value = sub->_default_value;
      _is_unsigned = sub->_is_unsigned;
      _is_int = sub->_is_int;
      _is_enum = true;
      /// _type_name, however, is set by ASTEnum::nud
      break;
    }
    case Ty::STRUCT: {
      /// align size is the max element size, if no element, 8 bits
      /// size is the number of elements * align size
      if (_is_forward_decl) {
        auto real = ast_cast<ASTTy>(_cs->get(_type_name));
        if (!real) { error("Incomplete type"); }
        *this = *real;
        _is_forward_decl = false;
      } else {
        _align_bits = 8;
        size_t n = _children.size();
        for (size_t i = 0; i < n; ++i) {
          auto et = ast_cast<ASTTy>(_children[i]);
          auto s = et->get_size_bits();
          if (s > _align_bits) { _align_bits = s; }
        }
        _size_bits = n * _align_bits;
        _is_struct = true;
      }
      break;
    }
    case Ty::ARRAY: {
      if (_children.empty()) { error("Invalid type"); }
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
      if (_children.empty()) { error("Invalid type"); }
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
      error("Invalid type");
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
  for (size_t i = 0; i < _n_elements; ++i) { _children.push_back(element->get_ty()); }
  /// set _type_name to [<element type>, <n_elements>]
  _type_name = "[" + _type_name + ", " + std::to_string(_n_elements) + "]";
  ++_end_index; /// skip "]"
  return _end_index;
}

size_t ASTTy::nud_struct() {
  _end_index = _start_index + 1; /// skip "struct"
  /// struct typename
  auto id = _parser->parse<ASTType::ID>(_end_index, true);
  TAN_ASSERT(id->is_named());
  _type_name = id->get_name();

  auto forward_decl = _cs->get(_type_name);
  if (!forward_decl) {
    _cs->add(_type_name, this->shared_from_this()); /// add self to current scope
  } else {
    /// replace forward decl with self (even if this is a forward declaration too)
    _cs->set(_type_name, this->shared_from_this());
  }

  /// struct body
  if (_parser->at(_end_index)->value == "{") {
    auto comp_stmt = _parser->next_expression(_end_index);
    if (!comp_stmt || comp_stmt->_type != ASTType::STATEMENT) { error(_end_index, "Invalid struct body"); }

    /// resolve member names and types
    auto members = comp_stmt->_children;
    ASTNodePtr var_decl = nullptr;
    size_t n = comp_stmt->_children.size();
    _member_names.reserve(n);
    _children.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      if (members[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
        var_decl = members[i];
        _children.push_back(var_decl->get_ty());
      } else if (members[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
        var_decl = members[i]->_children[0];
        auto initial_value = members[i]->_children[1];
        // TODO: check if value is compile-time known
        _children.push_back(initial_value->get_ty()); /// initial value is set to ASTTy in ASTLiteral::get_ty()
      } else { members[i]->error("Invalid struct member"); }
      auto name = var_decl->get_name();
      _member_names.push_back(name);
      _member_indices[name] = i;
    }
    this->resolve();
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
  resolve();
  return _end_index;
}

ASTTyPtr ASTTy::get_ptr_to() { return ASTTy::Create(Ty::POINTER, {get_ty()}, false); }

bool ASTTy::is_array() {
  resolve();
  return _is_array;
}

bool ASTTy::is_ptr() {
  resolve();
  return _is_ptr;
}

bool ASTTy::is_float() {
  resolve();
  return _is_float;
}

bool ASTTy::is_double() {
  resolve();
  return _is_double;
}

bool ASTTy::is_int() {
  resolve();
  return _is_int;
}

bool ASTTy::is_bool() {
  resolve();;
  return _is_bool;
}

bool ASTTy::is_enum() {
  resolve();;
  return _is_enum;
}

bool ASTTy::is_unsigned() {
  resolve();;
  return _is_unsigned;
}

bool ASTTy::is_struct() {
  resolve();;
  return _is_struct;
}

bool ASTTy::is_floating() {
  resolve();;
  return _is_float || _is_double;
}

bool ASTTy::is_lvalue() {
  resolve();;
  return _is_lvalue;
}

bool ASTTy::is_typed() { return true; }

size_t ASTTy::get_size_bits() {
  resolve();;
  return _size_bits;
}

str ASTTy::to_string(bool print_prefix) { return ASTNode::to_string(print_prefix) + " " + get_type_name(); }

ASTTy::ASTTy(Token *token, size_t token_index) : ASTNode(ASTType::TY, 0, 0, token, token_index) {}

void ASTTy::set_is_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }

bool ASTTy::operator!=(const ASTTy &other) { return !this->operator==(other); }

ASTTyPtr ASTTy::get_contained_ty() {
  if (_tyty == Ty::STRING) { return ASTTy::Create(Ty::CHAR, vector<ASTNodePtr>(), false); }
  else if (_is_ptr) {
    TAN_ASSERT(_children.size());
    auto ret = ast_cast<ASTTy>(_children[0]);
    TAN_ASSERT(ret);
    ret->resolve();
    return ret;
  } else { return nullptr; }
}

size_t ASTTy::get_n_elements() { return _n_elements; }

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
  _is_double = other._is_double;
  _is_int = other._is_int;
  _is_unsigned = other._is_unsigned;
  _is_struct = other._is_struct;
  _is_bool = other._is_bool;
  _is_enum = other._is_enum;
  _n_elements = other._n_elements;
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
  _is_double = other._is_double;
  _is_int = other._is_int;
  _is_unsigned = other._is_unsigned;
  _is_struct = other._is_struct;
  _is_bool = other._is_bool;
  _is_enum = other._is_enum;
  _n_elements = other._n_elements;
  _is_lvalue = other._is_lvalue;
  _is_forward_decl = other._is_forward_decl;
  return const_cast<ASTTy &>(*this);
}

ASTNodePtr ASTTy::get_member(size_t i) { return _children[i]; }

size_t ASTTy::get_member_index(str name) {
  if (_member_indices.find(name) == _member_indices.end()) {
    error("Unknown member of struct '" + get_type_name() + "'"); // TODO: move this outside
  }
  return _member_indices.at(name);
}

str ASTTy::get_type_name() {
  resolve();
  return _type_name;
}
