#include "token.h"
#include "ast_ty.h"
#include "compiler_session.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_expr.h"

namespace tanlang {

ASTTy::ASTTy(Token *token, size_t token_index) : ASTNode(ASTType::TY, 0, 0, token, token_index) {}

llvm::Type *ASTTy::to_llvm_type(CompilerSession *compiler_session) const {
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  llvm::Type *type = nullptr;
  /// primitive types
  switch (base) {
    case Ty::INT: {
      unsigned bits = 32;
      if (TY_IS(qual, Ty::BIT8)) {
        bits = 8;
      } else if (TY_IS(qual, Ty::BIT16)) {
        bits = 16;
      } else if (TY_IS(qual, Ty::BIT32)) {
        bits = 32;
      } else if (TY_IS(qual, Ty::BIT64)) {
        bits = 64;
      } else if (TY_IS(qual, Ty::BIT128)) {
        bits = 128;
      }
      type = compiler_session->get_builder()->getIntNTy(bits);
      break;
    }
    case Ty::BOOL: {
      type = compiler_session->get_builder()->getInt1Ty();
      break;
    }
    case Ty::FLOAT: {
      type = compiler_session->get_builder()->getFloatTy();
      break;
    }
    case Ty::DOUBLE: {
      type = compiler_session->get_builder()->getDoubleTy();
      break;
    }
    case Ty::STRING: {
      type = compiler_session->get_builder()->getInt8PtrTy(); /// str as char*
      break;
    }
    case Ty::VOID: {
      type = compiler_session->get_builder()->getVoidTy();
      break;
    }
    case Ty::STRUCT: {
      auto st = std::reinterpret_pointer_cast<ASTStruct>(compiler_session->get(_type_name));
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
      auto child = std::reinterpret_pointer_cast<ASTTy>(_children[0]);
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
  std::string type_name = "";
  unsigned size_bits = 0;
  unsigned encoding = 0;
  /// primitive types
  switch (base) {
    case Ty::INT: {
      size_bits = 32;
      type_name = "i32";
      if (TY_IS(qual, Ty::BIT8)) {
        size_bits = 8;
        type_name = "i8";
      } else if (TY_IS(qual, Ty::BIT16)) {
        size_bits = 16;
        type_name = "i16";
      } else if (TY_IS(qual, Ty::BIT64)) {
        size_bits = 64;
        type_name = "i64";
      } else if (TY_IS(qual, Ty::BIT128)) {
        size_bits = 128;
        type_name = "i128";
      }
      if (TY_IS(qual, Ty::UNSIGNED)) {
        if (size_bits == 8) {
          encoding = llvm::dwarf::DW_ATE_unsigned_char;
        } else {
          encoding = llvm::dwarf::DW_ATE_unsigned;
        }
      } else {
        if (size_bits == 8) {
          encoding = llvm::dwarf::DW_ATE_signed_char;
        } else {
          encoding = llvm::dwarf::DW_ATE_signed;
        }
      }
      ret = compiler_session->get_di_builder()->createBasicType(type_name, size_bits, encoding);
      break;
    }
    case Ty::BOOL: {
      type_name = "bool";
      size_bits = 8;
      encoding = llvm::dwarf::DW_ATE_boolean;
      ret = compiler_session->get_di_builder()->createBasicType(type_name, size_bits, encoding);
      break;
    }
    case Ty::FLOAT: {
      type_name = "float";
      size_bits = 32;
      encoding = llvm::dwarf::DW_ATE_float;
      ret = compiler_session->get_di_builder()->createBasicType(type_name, size_bits, encoding);
      break;
    }
    case Ty::DOUBLE: {
      type_name = "double";
      size_bits = 64;
      encoding = llvm::dwarf::DW_ATE_float;
      ret = compiler_session->get_di_builder()->createBasicType(type_name, size_bits, encoding);
      break;
    }
    case Ty::STRING: {
      auto *e_di_type = compiler_session->get_di_builder()->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      type_name = "u8*";
      size_bits = TAN_PTR_SIZE_BITS;
      unsigned align_bits = (unsigned) e_di_type->getSizeInBits();
      ret = compiler_session->get_di_builder()
          ->createPointerType(e_di_type, size_bits, align_bits, llvm::None, type_name);
      break;
    }
    case Ty::VOID: {
      type_name = "void";
      size_bits = 0;
      encoding = llvm::dwarf::DW_ATE_signed;
      compiler_session->get_di_builder()->createBasicType(type_name, size_bits, encoding);
      break;
    }
    case Ty::STRUCT: {
      DIFile *di_file = compiler_session->get_di_file();
      // TODO: align size in bits
      unsigned align_bits = 64;
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
              size_bits,
              align_bits,
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
      type_name = e->get_type_name() + "*";
      size_bits = TAN_PTR_SIZE_BITS;
      unsigned align_bits = e_di_type->getAlignInBits();
      ret = compiler_session->get_di_builder()
          ->createPointerType(e_di_type, size_bits, align_bits, llvm::None, type_name);
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
      type_name = child->get_type_name();
      ret = child->to_llvm_meta(compiler_session);
    }
    type_name += "*";
    size_bits = TAN_PTR_SIZE_BITS;
    unsigned align_bits = (unsigned) ret->getSizeInBits();
    ret = compiler_session->get_di_builder()->createPointerType(ret, size_bits, align_bits, llvm::None, type_name);
  }
  return ret;
}

std::string ASTTy::get_type_name() const {
  if (!_type_name.empty()) { return _type_name; }
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  /// primitive types
  switch (base) {
    case Ty::INT: {
      _type_name = "i32";
      if (TY_IS(qual, Ty::BIT8)) {
        _type_name = "i8";
      } else if (TY_IS(qual, Ty::BIT16)) {
        _type_name = "i16";
      } else if (TY_IS(qual, Ty::BIT64)) {
        _type_name = "i64";
      } else if (TY_IS(qual, Ty::BIT128)) {
        _type_name = "i128";
      }
      break;
    }
    case Ty::BOOL: {
      _type_name = "boolean";
      break;
    }
    case Ty::FLOAT: {
      _type_name = "float";
      break;
    }
    case Ty::DOUBLE: {
      _type_name = "double";
      break;
    }
    case Ty::STRING: {
      _type_name = "u8*";
      break;
    }
    case Ty::VOID: {
      _type_name = "void";
      break;
    }
    default: {
      assert(false);
    }
  }
  /// pointer
  if (TY_IS(qual, Ty::POINTER)) {
    if (!_children.empty()) { /// pointer to pointer (to ...)
      _type_name = _children[0]->get_type_name();
    }
    _type_name += "*";
  }
  return _type_name;
}

std::string ASTTy::to_string(bool print_prefix) const {
  return ASTNode::to_string(print_prefix) + " " + get_type_name();
}

} // namespace tanlang
