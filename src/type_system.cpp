#include "src/type_system.h"
#include "src/common.h"
#include "compiler_session.h"
#include "src/ast/ast_ty.h"
#include "src/llvm_include.h"

namespace tanlang {

Value *TypeSystem::ConvertTo(CompilerSession *cs, Type *dest, Value *val, bool is_lvalue, bool is_signed) {
  auto *orig = val->getType();
  auto *loaded = val;
  if (is_lvalue) { /// IMPORTANT
    loaded = cs->get_builder()->CreateLoad(val);
    TAN_ASSERT(orig->getNumContainedTypes() == 1);
    orig = orig->getContainedType(0); /// we only care the type of the rvalue of orig_val
  }
  bool is_pointer1 = orig->isPointerTy();
  bool is_pointer2 = dest->isPointerTy();
  size_t s1 = orig->getPrimitiveSizeInBits();
  size_t s2 = dest->getPrimitiveSizeInBits();
  /// early return if types are the same
  if (is_llvm_type_same(orig, dest)) { return loaded; };
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return cs->get_builder()->CreateBitCast(loaded, dest);
  } else if (orig->isIntegerTy() && dest->isIntegerTy() && s2 != 1) {
    /// different int types except dest is bool (1-bit int)
    if (s1 == s2) {
      return loaded;
    } else {
      return cs->get_builder()->CreateZExtOrTrunc(loaded, dest);
    }
  } else if (orig->isIntegerTy() && dest->isFloatingPointTy()) { /// int to float/double
    if (is_signed) {
      return cs->get_builder()->CreateUIToFP(loaded, dest);
    } else {
      return cs->get_builder()->CreateSIToFP(loaded, dest);
    }
  } else if (orig->isFloatingPointTy() && dest->isIntegerTy()) { /// float/double to int
    if (is_signed) {
      return cs->get_builder()->CreateFPToUI(loaded, dest);
    } else {
      return cs->get_builder()->CreateFPToSI(loaded, dest);
    }
  } else if (orig->isFloatingPointTy() && dest->isFloatingPointTy()) { /// float <-> double
    return cs->get_builder()->CreateFPCast(loaded, dest);
  } else if (dest->isIntegerTy(1)) { /// all types to bool, equivalent to val != 0
    if (orig->isFloatingPointTy()) {
      return cs->get_builder()->CreateFCmpONE(loaded, ConstantFP::get(cs->get_builder()->getFloatTy(), 0.0f));
    } else if (orig->isPointerTy()) {
      s1 = cs->get_ptr_size();
      loaded = cs->get_builder()->CreateIntToPtr(loaded, cs->get_builder()->getIntNTy((unsigned) s1));
      return cs->get_builder()
          ->CreateICmpNE(loaded, ConstantInt::get(cs->get_builder()->getIntNTy((unsigned) s1), 0, false));
    } else {
      return cs->get_builder()
          ->CreateICmpNE(loaded, ConstantInt::get(cs->get_builder()->getIntNTy((unsigned) s1), 0, false));
    }
  } else if (orig->isArrayTy() && dest->isArrayTy()) {
    /*
     * This should not be called, because:
     * - array type with size bound is checked during parsing phase
     * - all array types are treated as pointers during codegen phase
     * - even llvm::ArrayConstant is immediately converted to pointers after allocation
    */
    TAN_ASSERT(false);
  } else { throw std::runtime_error("Invalid type conversion"); }
}

DISubroutineType *create_function_type(CompilerSession *cs, Metadata *ret, vector<Metadata *> args) {
  vector<Metadata *> types{ret};
  types.reserve(args.size());
  types.insert(types.begin() + 1, args.begin(), args.end());
  return cs->get_di_builder()
      ->createSubroutineType(cs->get_di_builder()->getOrCreateTypeArray(types),
          DINode::FlagZero,
          llvm::dwarf::DW_CC_normal);
}

int TypeSystem::CanImplicitCast(ASTTyPtr t1, ASTTyPtr t2) {
  TAN_ASSERT(t1);
  TAN_ASSERT(t2);
  if (t1.get() == t2.get()) { return 0; } /// since some ASTTy are cached, compare pointer and early return
  if (*t1 == *t2) { return 0; }
  size_t s1 = t1->get_size_bits();
  size_t s2 = t2->get_size_bits();

  if (t1->is_bool()) { return 0; }
  else if (t2->is_bool()) { return 1; }
  else if (t1->is_int() && t2->is_int()) { /// between integers
    /// should be both unsigned or both signed
    if (t1->is_unsigned() ^ t2->is_unsigned()) { return -1; }
    return s1 >= s2 ? 0 : 1;
  } else if (t1->is_floating() && t2->is_int()) { /// float/double and int
    return 0;
  } else if (t1->is_int() && t2->is_floating()) { /// int and float/double
    return 1;
  } else if (t1->is_floating() && t2->is_floating()) { /// float/double and float/double
    return s1 >= s2 ? 0 : 1;
  } else if (t1->is_array() && t2->is_array()) { /// arrays
    /// array size must be the same
    if (t1->get_n_elements() != t2->get_n_elements()) { return -1; }
    /// the element type can be implicitly casted as long as the elements have the same size
    if (t1->get_contained_ty()->get_size_bits() != t2->get_contained_ty()->get_size_bits()) { return -1; }
    return CanImplicitCast(t1->get_contained_ty(), t2->get_contained_ty());
  }
  return -1;
}

} // namespace tanlang
