#include "src/ast/ast_ty.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_number_literal.h"
#include "parser.h"
#include "token.h"
#include "compiler_session.h"
#include "compiler.h"

namespace tanlang {

std::unordered_map<Ty, ASTTyPtr> ASTTy::_cached{};

std::shared_ptr<ASTTy> ASTTy::Create(Ty t, bool is_lvalue, std::vector<std::shared_ptr<ASTTy>> sub_tys) {
  // FIXME: do NOT use Ty as keys
  /*
  if (ASTTy::_cached.find(t) != ASTTy::_cached.end() && t != Ty::ARRAY && t != Ty::POINTER) {
    return ASTTy::_cached[t];
  } else {
   */
  auto ret = std::make_shared<ASTTy>(nullptr, 0);
  ret->_ty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  ret->resolve();
  // ASTTy::_cached[t] = ret;
  return ret;
  /*
  }
   */
}

ASTTy::ASTTy(Token *token, size_t token_index) : ASTNode(ASTType::TY, 0, 0, token, token_index) {}

llvm::Value *ASTTy::get_llvm_value(CompilerSession *cs) const {
  Ty base = TY_GET_BASE(_ty);
  Value *ret = nullptr;
  Type *type = this->to_llvm_type(cs);
  /// primitive types
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
      assert(false);
      break;
    case Ty::STRUCT: {
      auto st = ast_cast<ASTStruct>(cs->get(_type_name));
      assert(st);
      ret = st->get_llvm_value(cs);
      break;
    }
    case Ty::POINTER:
      ret = ConstantPointerNull::get((PointerType *) type);
      break;
    case Ty::ARRAY: {
      // TODO: default value of arrays
      break;
    }
    default:
      assert(false);
  }
  return ret;
}

llvm::Type *ASTTy::to_llvm_type(CompilerSession *compiler_session) const {
  Ty base = TY_GET_BASE(_ty);
  llvm::Type *type = nullptr;
  /// primitive types
  switch (base) {
    case Ty::INT:
      type = compiler_session->get_builder()->getIntNTy((unsigned) _size_bits);
      break;
    case Ty::CHAR:
      type = compiler_session->get_builder()->getInt8Ty();
      break;
    case Ty::BOOL:
      type = compiler_session->get_builder()->getInt1Ty();
      break;
    case Ty::FLOAT:
      type = compiler_session->get_builder()->getFloatTy();
      break;
    case Ty::DOUBLE:
      type = compiler_session->get_builder()->getDoubleTy();
      break;
    case Ty::STRING:
      type = compiler_session->get_builder()->getInt8PtrTy(); /// str as char*
      break;
    case Ty::VOID:
      type = compiler_session->get_builder()->getVoidTy();
      break;
    case Ty::STRUCT: {
      /// ASTStruct must override this, otherwise ASTStruct must override this, otherwise ASTStruct must override ...
      auto st = ast_cast<ASTStruct>(compiler_session->get(_type_name));
      type = st->to_llvm_type(compiler_session);
      break;
    }
    case Ty::POINTER:
    case Ty::ARRAY: {
      auto e_type = ast_cast<ASTTy>(_children[0])->to_llvm_type(compiler_session);
      type = e_type->getPointerTo();
      break;
    }
    default:
      assert(false);
  }
  return type;
}

llvm::DIType *ASTTy::to_llvm_meta(CompilerSession *compiler_session) const {
  Ty base = TY_GET_BASE(_ty);
  // TODO: Ty qual = TY_GET_QUALIFIER(_ty);
  DIType *ret = nullptr;
  /// primitive types
  switch (base) {
    case Ty::CHAR:
    case Ty::INT:
    case Ty::BOOL:
    case Ty::FLOAT:
    case Ty::VOID:
    case Ty::DOUBLE:
      ret = compiler_session->get_di_builder()->createBasicType(_type_name, _size_bits, _dwarf_encoding);
      break;
    case Ty::STRING: {
      auto *e_di_type = compiler_session->get_di_builder()->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = compiler_session->get_di_builder()
          ->createPointerType(e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    case Ty::STRUCT: {
      DIFile *di_file = compiler_session->get_di_file();
      auto st = compiler_session->get(_type_name);
      size_t n = st->_children.size();
      std::vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        auto e = st->_children[i]; // ASTVarDecl
        elements.push_back(ast_cast<ASTTy>(e->_children[1])->to_llvm_meta(compiler_session));
      }
      ret = compiler_session->get_di_builder()
          ->createStructType(compiler_session->get_current_di_scope(),
              _type_name,
              di_file,
              (unsigned) st->_token->l,
              _size_bits,
              (unsigned) _align_bits,
              DINode::DIFlags::FlagZero,
              nullptr,
              compiler_session->get_di_builder()->getOrCreateArray(elements),
              0,
              nullptr,
              _type_name);
      break;
    }
    case Ty::POINTER:
    case Ty::ARRAY: {
      auto e = ast_cast<ASTTy>(_children[0]);
      auto *e_di_type = e->to_llvm_meta(compiler_session);
      ret = compiler_session->get_di_builder()
          ->createPointerType(e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    default:
      assert(false);
  }
  return ret;
}

std::string ASTTy::get_type_name() const {
  assert(!_type_name.empty());
  return _type_name;
}

std::string ASTTy::to_string(bool print_prefix) const {
  return ASTNode::to_string(print_prefix) + " " + get_type_name();
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

bool ASTTy::operator!=(const ASTTy &other) const { return !this->operator==(other); }

void ASTTy::resolve() {
  if (_resolved) { return; }
  auto *tm = Compiler::GetDefaultTargetMachine();
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  /// primitive types
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
      } else if (TY_IS(qual, Ty::BIT128)) {
        // TODO: remove i128 and u128
        assert(false);
        _size_bits = 128;
        _type_name = "i128";
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
    case Ty::STRUCT:
      // TODO: align size in bits
      _align_bits = 64;
      _is_struct = true;
      break;
    case Ty::POINTER:
    case Ty::ARRAY: {
      auto e = ast_cast<ASTTy>(_children[0]);
      assert(e);
      e->resolve();
      _type_name = e->get_type_name() + "*";
      _size_bits = tm->getPointerSizeInBits(0);
      _align_bits = e->get_size_bits();
      _is_ptr = true;
      _dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    default:
      assert(false);
  }
  _n_elements = _children.size();
  _resolved = true;
}

bool ASTTy::is_ptr() const {
  assert(_resolved);
  return _is_ptr;
}

bool ASTTy::is_float() const {
  assert(_resolved);
  return _is_float;
}

bool ASTTy::is_double() const {
  assert(_resolved);
  return _is_double;
}

bool ASTTy::is_int() const {
  assert(_resolved);
  return _is_int;
}

bool ASTTy::is_bool() const {
  assert(_resolved);
  return _is_bool;
}

bool ASTTy::is_unsigned() const {
  assert(_resolved);
  return _is_unsigned;
}

bool ASTTy::is_struct() const {
  assert(_resolved);
  return _is_struct;
}

bool ASTTy::is_floating() const {
  assert(_resolved);
  return _is_float || _is_double;
}

bool ASTTy::is_lvalue() const { return _is_lvalue; }

bool ASTTy::is_typed() const { return true; }

size_t ASTTy::get_size_bits() const { return _size_bits; }

void ASTTy::set_is_lvalue(bool is_lvalue) { _is_lvalue = is_lvalue; }

std::unordered_map<std::string, Ty> basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
        {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
        {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
        {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
        {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"i128", TY_OR(Ty::INT, Ty::BIT128)},
        {"u128", TY_OR3(Ty::INT, Ty::BIT128, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING},
        {"char", Ty::CHAR}, {"bool", Ty::BOOL},};

std::unordered_map<std::string, Ty>
    qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

/// current token should be "[" when this is called
/// this will set _type_name
size_t ASTTy::nud_array() {
  _end_index = _start_index + 1; /// skip "["
  /// element type
  if (_parser->at(_end_index)->value == "]") { /// empty
    report_code_error(_parser->at(_end_index), "The array type and size must be specified");
  } else {
    auto child = std::make_shared<ASTTy>(_parser->at(_end_index), _end_index);
    _end_index = child->parse(_parser, _cs); /// this set the _type_name of child
    _children.push_back(child);
    _type_name = child->_type_name;
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
      _ty = TY_OR(_ty, basic_tys[token->value]);
      _type_name += token->value; /// just append the type name for basic types and qualifiers
    } else if (qualifier_tys.find(token->value) != qualifier_tys.end()) { /// qualifiers
      if (token->value == "*") {
        if (_ty == Ty::INVALID) { /// pointer to basic types
          _ty = Ty::POINTER;
        } else { /// pointer to pointer (to ...)
          auto sub = std::make_shared<ASTTy>(token, _end_index);
          // swap self and child, so this is a pointer with no basic type, and the child is a pointer to something
          sub->_ty = this->_ty;
          this->_ty = Ty::POINTER;
          sub->_type_name = _type_name;
          _children.push_back(sub);
        }
      }
    } else if (token->type == TokenType::ID) { /// struct or array
      // TODO: identify type aliases
      _type_name = token->value; /// _type_name is the name of the struct
      _ty = TY_OR(_ty, Ty::STRUCT);
    } else if (token->value == "[") {
      _ty = TY_OR(_ty, Ty::ARRAY);
      _end_index = nud_array(); /// set _type_name in nud_array()
    } else { break; }
    ++_end_index;
  }
  resolve(); /// fill in relevant member variables
  return _end_index;
}

ASTTyPtr ASTTy::get_contained_ty() const {
  if (_ty == Ty::STRING) { return ASTTy::Create(Ty::CHAR); }
  else if (_is_ptr) {
    assert(_children.size());
    auto ret = ast_cast<ASTTy>(_children[0]);
    assert(ret);
    return ret;
  } else { return nullptr; }
}

std::shared_ptr<ASTTy> ASTTy::get_ty() const { return std::const_pointer_cast<ASTTy>(this->shared_from_this()); }

} // namespace tanlang
