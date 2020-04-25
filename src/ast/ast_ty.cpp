#include "token.h"
#include "ast_ty.h"
#include "compiler_session.h"
#include "src/ast/ast_struct.h"

namespace tanlang {

ASTTy::ASTTy(Token *token, size_t token_index) : ASTNode(ASTType::TY, 0, 0, token, token_index) {}

llvm::Type *ASTTy::to_llvm_type(CompilerSession *compiler_session) const {
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  llvm::Type *type = nullptr;
  /// primitive types
  switch (base) {
    case Ty::INT:
      type = compiler_session->get_builder()->getIntNTy((unsigned) _size_bits);
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
      auto st = ast_cast<ASTStruct>(compiler_session->get(_type_name));
      type = st->to_llvm_type(compiler_session);
      break;
    }
    case Ty::ARRAY: {
      auto e_type = ast_cast<ASTTy>(_children[0])->to_llvm_type(compiler_session);
      type = e_type->getPointerTo();
      break;
    }
    default: {
      break;
    }
  }
  /// pointer
  if (TY_IS(qual, Ty::POINTER)) {
    if (!_children.empty()) { /// pointer to pointer (to ...)
      auto child = ast_cast<ASTTy>(_children[0]);
      type = child->to_llvm_type(compiler_session);
    }
    type = llvm::PointerType::get(type, 0);
  }
  return type;
}

llvm::DIType *ASTTy::to_llvm_meta(CompilerSession *compiler_session) const {
  // TODO: debug info for qualifiers
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  DIType *ret = nullptr;
  /// primitive types
  switch (base) {
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
    case Ty::ARRAY: {
      auto e = ast_cast<ASTTy>(_children[0]);
      auto *e_di_type = e->to_llvm_meta(compiler_session);
      ret = compiler_session->get_di_builder()
          ->createPointerType(e_di_type, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
      break;
    }
    default: {
      break;
    }
  }
  /// pointer
  if (TY_IS(qual, Ty::POINTER)) {
    if (!_children.empty()) { /// pointer to pointer (to ...)
      auto child = ast_cast<ASTTy>(_children[0]);
      ret = child->to_llvm_meta(compiler_session);
    }
    ret = compiler_session->get_di_builder()
        ->createPointerType(ret, _size_bits, (unsigned) _align_bits, llvm::None, _type_name);
  }
  return ret;
}

std::string ASTTy::get_type_name() const {
  assert (!_type_name.empty());
  return _type_name;
}

std::string ASTTy::to_string(bool print_prefix) const {
  return ASTNode::to_string(print_prefix) + " " + get_type_name();
}

std::shared_ptr<ASTTy> ASTTy::Create(Ty t, bool is_lvalue, std::vector<std::shared_ptr<ASTTy>> sub_tys) {
  auto ret = std::make_shared<ASTTy>(nullptr, 0);
  ret->_ty = t;
  ret->_is_lvalue = is_lvalue;
  ret->_children.insert(ret->_children.begin(), sub_tys.begin(), sub_tys.end());
  ret->_n_elements = sub_tys.size();
  ret->resolve();
  return ret;
}

bool ASTTy::operator==(const ASTTy &other) const {
  if (_ty != other._ty) {
    return false;
  }

  if (!_children.empty()) {
    size_t n = _children.size();
    if (n != other._children.size()) {
      return false;
    }
    for (size_t i = 0; i < n; ++i) {
      return ast_cast<ASTTy>(_children[i])->operator==(*ast_cast<ASTTy>(other._children[i]));
    }
  }
  return true;
}

void ASTTy::resolve() {
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  /// primitive types
  switch (base) {
    case Ty::INT: {
      _size_bits = 32;
      _type_name = "i32";
      _is_int = true;
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
    case Ty::BOOL: {
      _type_name = "bool";
      _size_bits = 8;
      _dwarf_encoding = llvm::dwarf::DW_ATE_boolean;
      break;
    }
    case Ty::FLOAT: {
      _type_name = "float";
      _size_bits = 32;
      _dwarf_encoding = llvm::dwarf::DW_ATE_float;
      _is_float = true;
      break;
    }
    case Ty::DOUBLE: {
      _type_name = "double";
      _size_bits = 64;
      _dwarf_encoding = llvm::dwarf::DW_ATE_float;
      _is_double = true;
      break;
    }
    case Ty::STRING: {
      _type_name = "u8*";
      _size_bits = 64;// TODO: compiler_session->get_ptr_size();
      _align_bits = 8;
      _is_ptr = true;
      break;
    }
    case Ty::VOID: {
      _type_name = "void";
      _size_bits = 0;
      _dwarf_encoding = llvm::dwarf::DW_ATE_signed;
      break;
    }
    case Ty::STRUCT: {
      // TODO: align size in bits
      _align_bits = 64;
      _is_struct = true;
      break;
    }
    case Ty::ARRAY: {
      auto e = ast_cast<ASTTy>(_children[0]);
      _type_name = e->get_type_name() + "*";
      _size_bits = 64; // TODO: compiler_session->get_ptr_size();
      _align_bits = e->get_size_bits();
      _is_ptr = true;
      break;
    }
    default: {
      break;
    }
  }
  /// pointer
  if (TY_IS(qual, Ty::POINTER)) {
    _is_ptr = true;
    if (!_children.empty()) { /// pointer to pointer (to ...)
      auto child = ast_cast<ASTTy>(_children[0]);
      child->resolve();
      _type_name = child->_type_name + "*";
      _align_bits = child->_size_bits;
      _size_bits = 64; // TODO: compiler_session->get_ptr_size();
      _dwarf_encoding = llvm::dwarf::DW_ATE_address;
    } else {
      _type_name += "*";
      _align_bits = _size_bits;
      _size_bits = 64; // TODO: compiler_session->get_ptr_size();
      _dwarf_encoding = llvm::dwarf::DW_ATE_address;
    }
  }
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

int ASTTy::CanImplicitCast(ASTTyPtr t1, ASTTyPtr t2) {
  if (*t1 == *t2) { return 0; }
  size_t s1 = t1->get_size_bits();
  size_t s2 = t2->get_size_bits();
  if (t1->is_ptr() && t2->is_ptr()) { /// both pointer
    // TODO: check if safe to cast
    return 0;
  } else if (t1->is_int() && t2->is_int()) { /// between integers
    return s1 >= s2 ? 0 : 1;
  } else if (t1->is_floating() && t2->is_int()) { /// float/double and int
    return 0;
  } else if (t1->is_int() && t2->is_floating()) { /// int and float/double
    return 1;
  } else if (t1->is_floating() && t2->is_floating()) { /// float/double and float/double
    return s1 >= s2 ? 0 : 1;
  }
  // TODO: complex type
  return -1;
}

} // namespace tanlang
