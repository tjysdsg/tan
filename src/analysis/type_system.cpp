#include "src/analysis/type_system.h"
#include "src/analysis/analysis.h"
#include "src/common.h"
#include "compiler_session.h"
#include "src/ast/ast_ty.h"

namespace tanlang {

llvm::Value *TypeSystem::ConvertTo(CompilerSession *cs, llvm::Value *val, ASTTyPtr orig, ASTTyPtr dest) {
  auto *builder = cs->_builder;
  auto *loaded = val;

  /// load if lvalue
  if (orig->_is_lvalue) { loaded = builder->CreateLoad(val); }

  bool is_pointer1 = orig->_is_ptr;
  bool is_pointer2 = dest->_is_ptr;
  size_t s1 = orig->_size_bits;

  /**
   * NOTE: check enum before checking int
   * */

  /// early return if types are the same
  if (*orig == *dest) { return loaded; };
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return builder->CreateBitCast(loaded, dest->to_llvm_type(cs));
  } else if ((orig->_is_enum && dest->_is_int) || (dest->_is_enum && orig->_is_int)) {
    return builder->CreateZExtOrTrunc(loaded, dest->to_llvm_type(cs));
  } else if (orig->_is_int && dest->_is_int) {
    return builder->CreateZExtOrTrunc(loaded, dest->to_llvm_type(cs));
  } else if (orig->_is_int && dest->_is_float) { /// int to float/double
    if (orig->_is_unsigned) {
      return builder->CreateUIToFP(loaded, dest->to_llvm_type(cs));
    } else {
      return builder->CreateSIToFP(loaded, dest->to_llvm_type(cs));
    }
  } else if (orig->_is_float && dest->_is_int) { /// float/double to int
    if (orig->_is_unsigned) {
      return builder->CreateFPToUI(loaded, dest->to_llvm_type(cs));
    } else {
      return builder->CreateFPToSI(loaded, dest->to_llvm_type(cs));
    }
  } else if (orig->_is_float && dest->_is_float) { /// float <-> double
    return builder->CreateFPCast(loaded, dest->to_llvm_type(cs));
  } else if (orig->_is_bool && dest->_is_int) { /// bool to int
    return builder->CreateZExtOrTrunc(val, dest->to_llvm_type(cs));
  } else if (dest->_is_bool) { /// all types to bool, equivalent to val != 0
    if (orig->_is_float) {
      return builder->CreateFCmpONE(loaded, ConstantFP::get(builder->getFloatTy(), 0.0f));
    } else if (orig->_is_ptr) {
      s1 = cs->get_ptr_size();
      loaded = builder->CreateIntToPtr(loaded, builder->getIntNTy((unsigned) s1));
      return builder->CreateICmpNE(loaded, ConstantInt::get(builder->getIntNTy((unsigned) s1), 0, false));
    } else {
      return builder->CreateICmpNE(loaded, ConstantInt::get(builder->getIntNTy((unsigned) s1), 0, false));
    }
  } else if (orig->_is_array && dest->_is_array) {
    // FIXME: casting array of float to/from array of integer is broken
    TAN_ASSERT(false);
  } else {
    // TODO: move this outside
    report_error("Invalid type conversion");
  }
}

DISubroutineType *create_function_type(CompilerSession *cs, Metadata *ret, vector<Metadata *> args) {
  vector<Metadata *> types{ret};
  types.reserve(args.size());
  types.insert(types.begin() + 1, args.begin(), args.end());
  return cs->_di_builder
      ->createSubroutineType(cs->_di_builder->getOrCreateTypeArray(types), DINode::FlagZero, llvm::dwarf::DW_CC_normal);
}

int TypeSystem::CanImplicitCast(CompilerSession *cs, ASTTyPtr t1, ASTTyPtr t2) {
  TAN_ASSERT(t1);
  TAN_ASSERT(t2);
  if (t1.get() == t2.get()) { return 0; } /// since some ASTTy are cached, compare pointer and early return
  if (*t1 == *t2) { return 0; }
  size_t s1 = t1->_size_bits;
  size_t s2 = t2->_size_bits;

  if (t1->_is_ptr && t2->_is_ptr && *get_contained_ty(cs, t1) == *get_contained_ty(cs, t2)) {
    return 0;
  } else if (t1->_is_bool) { return 0; }
  else if (t2->_is_bool) { return 1; }
  else if (t1->_is_enum && t2->_is_int) {
    return 1;
  } else if (t2->_is_enum && t1->_is_int) {
    return 0;
  } else if (t1->_is_int && t2->_is_int) { /// between integers
    /// should be both unsigned or both signed
    if (t1->_is_unsigned ^ t2->_is_unsigned) { return -1; }
    return s1 >= s2 ? 0 : 1;
  } else if (t1->_is_float && t2->_is_int) { /// float/double and int
    return 0;
  } else if (t1->_is_int && t2->_is_float) { /// int and float/double
    return 1;
  } else if (t1->_is_float && t2->_is_float) { /// float/double and float/double
    return s1 >= s2 ? 0 : 1;
  } else if (t1->_is_array && t2->_is_array) { /// arrays
    /// array size must be the same
    if (get_n_children(t1) != get_n_children(t2)) { return -1; }
    /// the element type can be implicitly casted as long as the elements have the same size
    if (get_contained_ty(cs, t1)->_size_bits != get_contained_ty(cs, t2)->_size_bits) { return -1; }
    return CanImplicitCast(cs, get_contained_ty(cs, t1), get_contained_ty(cs, t2));
  }
  return -1;
}

} // namespace tanlang
