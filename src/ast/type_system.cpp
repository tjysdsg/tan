#include "type_system.h"
#include "compiler_session.h"
#include "src/ast/common.h"

namespace tanlang {

llvm::Value *convert_to(CompilerSession *compiler_session,
    llvm::Type *dest,
    llvm::Value *orig_val,
    bool is_lvalue,
    bool is_signed) {
  auto *orig = orig_val->getType();
  auto *loaded = orig_val;
  if (is_lvalue) { /// IMPORTANT
    loaded = compiler_session->get_builder()->CreateLoad(orig_val);
    assert(orig->getNumContainedTypes() == 1);
    orig = orig->getContainedType(0); /// we only care the type of the rvalue of orig_val
  }
  bool is_pointer1 = orig->isPointerTy();
  bool is_pointer2 = dest->isPointerTy();
  unsigned s1 = orig->getPrimitiveSizeInBits();
  unsigned s2 = dest->getPrimitiveSizeInBits();
  /// early return if types are the same
  if (is_llvm_type_same(orig, dest)) { return loaded; };
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return compiler_session->get_builder()->CreateBitCast(loaded, dest);
  } else if (is_pointer1) { /// pointers to primitive types
    return loaded;
  } else if (is_pointer2 && is_lvalue) { /// primitive types to pointers
    return orig_val;
  } else if (orig->isIntegerTy() && dest->isIntegerTy() && s2 != 1) {
    /// different int types except dest is bool (1-bit int)
    if (s1 == s2) {
      return loaded;
    } else {
      return compiler_session->get_builder()->CreateZExtOrTrunc(loaded, dest);
    }
  } else if (orig->isIntegerTy() && dest->isFloatingPointTy()) { /// int to float/double
    if (is_signed) {
      return compiler_session->get_builder()->CreateUIToFP(loaded, dest);
    } else {
      return compiler_session->get_builder()->CreateSIToFP(loaded, dest);
    }
  } else if (orig->isFloatingPointTy() && dest->isIntegerTy()) { /// float/double to int
    if (is_signed) {
      return compiler_session->get_builder()->CreateFPToUI(loaded, dest);
    } else {
      return compiler_session->get_builder()->CreateFPToSI(loaded, dest);
    }
  } else if (orig->isFloatingPointTy() && dest->isFloatingPointTy()) { /// float <-> double
    return compiler_session->get_builder()->CreateFPCast(loaded, dest);
  } else if (dest->isIntegerTy(1)) { /// all types to bool, equivalent to val != 0
    // FIXME? floating point
    return compiler_session->get_builder()
        ->CreateICmpNE(loaded, ConstantInt::get(compiler_session->get_builder()->getIntNTy(s1), 0, false));
  } else {
    // TODO: complex type
    throw std::runtime_error("Invalid type conversion");
  }
}

int should_cast_to_which(CompilerSession *compiler_session, llvm::Type *t1, llvm::Type *t2) {
  if (is_llvm_type_same(t1, t2)) { return 0; }
  unsigned s1 = t1->getPrimitiveSizeInBits();
  unsigned s2 = t2->getPrimitiveSizeInBits();
  if (t1->isPointerTy() && t2->isPointerTy()) { /// both pointer, yes
    return 0;
  } else if (t1->isIntegerTy() && t2->isIntegerTy()) { /// between integers
    return s1 >= s2 ? 0 : 1;
  } else if (t1->isFloatingPointTy() && t2->isIntegerTy()) { /// float/double and int
    return 0;
  } else if (t1->isIntegerTy() && t2->isFloatingPointTy()) { /// int and float/double
    return 1;
  } else if (t1->isFloatingPointTy() && t2->isFloatingPointTy()) { /// float/double and float/double
    return s1 >= s2 ? 0 : 1;
  }
  // TODO: complex type
  return -1;
}

DISubroutineType *create_function_type(CompilerSession *compiler_session, Metadata *ret, std::vector<Metadata *> args) {
  std::vector<Metadata *> types{ret};
  types.reserve(args.size());
  types.insert(types.begin() + 1, args.begin(), args.end());
  return compiler_session->get_di_builder()
      ->createSubroutineType(compiler_session->get_di_builder()->getOrCreateTypeArray(types),
          DINode::FlagZero,
          llvm::dwarf::DW_CC_normal);
}

} // namespace
