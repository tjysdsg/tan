#include "ast_ty.h"
#include "compiler_session.h"
#include "src/llvm_include.h"
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
    case Ty::BOOL:
      type = compiler_session->get_builder()->getInt1Ty();
      break;
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

std::string ASTTy::get_type_name() const {
  return _type_name;
}

std::string ASTTy::to_string(bool print_prefix) const {
  return ASTNode::to_string(print_prefix) + " " + _type_name;
}

llvm::Value *ASTTy::convert_to(CompilerSession *compiler_session,
                               std::shared_ptr<ASTTy> dest_ty,
                               llvm::Value *orig_val) {
  // FIXME? check if orig_val->getType() == this->to_llvm_type()
  // TODO: early return if types are the same
  Ty target_ty = dest_ty->_ty;
  bool is_pointer1 = TY_IS(_ty, Ty::POINTER) || TY_IS(_ty, Ty::STRING);
  bool is_pointer2 = TY_IS(target_ty, Ty::POINTER) || TY_IS(target_ty, Ty::STRING);
  Type *target_llvm_type = dest_ty->to_llvm_type(compiler_session);
  Type *this_llvm_type = to_llvm_type(compiler_session);
  auto *loaded = orig_val;
  if (_is_lvalue) { /// IMPORTANT
    loaded = compiler_session->get_builder()->CreateLoad(orig_val);
  }
  unsigned s1 = this_llvm_type->getPrimitiveSizeInBits();
  unsigned s2 = target_llvm_type->getPrimitiveSizeInBits();
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return compiler_session->get_builder()->CreateBitCast(loaded, target_llvm_type);
  } else if (is_pointer1) { /// pointers to primitive types
    return compiler_session->get_builder()->CreateLoad(loaded);
  } else if (is_pointer2 && _is_lvalue) { /// primitive types to pointers, only possible for heap allocated
    return orig_val;
  } else if (!TY_IS(_ty, Ty::INT) && TY_IS(target_ty, Ty::INT)) { /// different int types
    if (s1 != s2) {
      return loaded;
    } else {
      return compiler_session->get_builder()->CreateZExtOrTrunc(loaded, target_llvm_type);
    }
  } else if (!TY_IS(_ty, Ty::INT) && TY_IS(target_ty, Ty::FLOAT)) { /// int to float
    if (TY_IS(_ty, Ty::UNSIGNED)) {
      return compiler_session->get_builder()->CreateUIToFP(loaded, target_llvm_type);
    } else {
      return compiler_session->get_builder()->CreateSIToFP(loaded, target_llvm_type);
    }
  } else if (!TY_IS(_ty, Ty::FLOAT) && TY_IS(target_ty, Ty::INT)) { /// float to int
    if (TY_IS(target_ty, Ty::UNSIGNED)) {
      return compiler_session->get_builder()->CreateFPToUI(loaded, target_llvm_type);
    } else {
      return compiler_session->get_builder()->CreateFPToSI(loaded, target_llvm_type);
    }
  } else if (!TY_IS(_ty, Ty::INT) && TY_IS(target_ty, Ty::DOUBLE)) { /// int to double
    throw std::runtime_error("Not implemented");
  } else if (!TY_IS(_ty, Ty::DOUBLE) && TY_IS(target_ty, Ty::INT)) { /// double to int
    throw std::runtime_error("Not implemented");
  } else if (!TY_IS(_ty, Ty::FLOAT) && TY_IS(target_ty, Ty::DOUBLE)) { /// float to double
    throw std::runtime_error("Not implemented");
  } else if (!TY_IS(_ty, Ty::DOUBLE) && TY_IS(target_ty, Ty::FLOAT)) { /// double to float
    throw std::runtime_error("Not implemented");
  } else if (TY_IS(target_ty, Ty::BOOL)) { /// all types to bool, equivalent to val != 0
    // FIXME? floating point
    return compiler_session->get_builder()
                           ->CreateICmpNE(loaded,
                                          ConstantInt::get(compiler_session->get_builder()->getIntNTy(s1), 0, false));
  }
  throw std::runtime_error(
      "Invalid type conversion from " + this->to_string(false) + " to " + dest_ty->to_string(false));
}

} // namespace tanlang
