#include "ast_ty.h"
#include "compiler_session.h"
#include "src/llvm_include.h"
#include "src/ast/ast_struct.h"

namespace tanlang {

ASTTy::ASTTy(Token *token) : ASTNode(ASTType::TY, 0, 0, token) {}

llvm::Type *ASTTy::to_llvm_type(CompilerSession *compiler_session) const {
  Ty base = TY_GET_BASE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  llvm::Type *type = nullptr;
  // primitive types
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
    case Ty::FLOAT: {
      type = compiler_session->get_builder()->getFloatTy();
      break;
    }
    case Ty::DOUBLE: {
      type = compiler_session->get_builder()->getDoubleTy();
      break;
    }
    case Ty::STRING: {
      type = compiler_session->get_builder()->getInt8Ty(); // LLVM stores string literals as int8*
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
      type = ArrayType::get(e_type, _n_elements);
      break;
    }
    default: {
      /// Base type could be 0 because this might be a pointer to pointer, but other than that, the base type must
      /// be non-zero
      if (_children.empty()) {
        throw std::runtime_error("Invalid base type" + std::to_string((uint64_t) base));
      }
      break;
    }
  }
  // pointer
  if (TY_IS(qual, Ty::POINTER)) {
    if (!_children.empty()) { // pointer to pointer (to ...)
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

} // namespace tanlang
