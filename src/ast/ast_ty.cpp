#include "ast_ty.h"
#include "compiler_session.h"
#include "src/llvm_include.h"

namespace tanlang {

ASTTy::ASTTy(Token *token) : ASTNode(ASTType::TY, 0, 0, token) {}

llvm::Type *ASTTy::to_llvm_type(CompilerSession *compiler_session) const {
  Ty base = TY_GET_BASE(_ty);
  Ty comp = TY_GET_COMPOSITE(_ty);
  Ty qual = TY_GET_QUALIFIER(_ty);
  if (TY_HAS_COMPOSITE(_ty)) {
    // TODO
  }
  llvm::Type *type = nullptr;

  if (!_children.empty()) { // pointer to pointer (to ...)
    auto child = std::reinterpret_pointer_cast<ASTTy>(_children[0]);
    type = child->to_llvm_type(compiler_session);
  } else {
    // base type with qualifier
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
        // TODO
        break;
      }
      case Ty::VOID: {
        type = compiler_session->get_builder()->getVoidTy();
        break;
      }
      default: { throw std::runtime_error("Invalid base type: " + std::to_string((uint64_t) base)); }
    }
  }
  // pointer
  if (TY_IS(qual, Ty::POINTER)) {
    type = llvm::PointerType::get(type, 0);
  }
  return type;
}

} // namespace tanlang
