#include "src/analysis/type_system.h"
#include "src/common.h"
#include "src/ast/constructor.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/ast/type.h"
#include "src/ast/ast_context.h"
#include "src/ast/expr.h"

using namespace tanlang;

Value *TypeSystem::ConvertTo(CompilerSession *cs, Expr *expr, Type *dest) {
  auto *builder = cs->_builder;

  /// load if lvalue
  Value *loaded = TypeSystem::LoadIfLValue(cs, expr);

  Type *orig = expr->get_type();

  bool is_pointer1 = orig->is_pointer();
  bool is_pointer2 = dest->is_pointer();

  /**
   * NOTE: check enum before checking int
   * */

  /// early return if types are the same
  if (orig == dest) { return loaded; };
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return builder->CreateBitCast(loaded, ToLLVMType(cs, dest));
  } else if ((orig->is_enum() && dest->is_int()) || (dest->is_enum() && orig->is_int())) {
    return builder->CreateZExtOrTrunc(loaded, ToLLVMType(cs, dest));
  } else if ((orig->is_int() || orig->is_char()) && (dest->is_char() || dest->is_int())) { /// between int
    return builder->CreateZExtOrTrunc(loaded, ToLLVMType(cs, dest));
  } else if (orig->is_int() && dest->is_float()) { /// int to float/double
    if (orig->is_unsigned()) {
      return builder->CreateUIToFP(loaded, ToLLVMType(cs, dest));
    } else {
      return builder->CreateSIToFP(loaded, ToLLVMType(cs, dest));
    }
  } else if (orig->is_float() && dest->is_int()) { /// float/double to int
    if (orig->is_unsigned()) {
      return builder->CreateFPToUI(loaded, ToLLVMType(cs, dest));
    } else {
      return builder->CreateFPToSI(loaded, ToLLVMType(cs, dest));
    }
  } else if (orig->is_float() && dest->is_float()) { /// float <-> double
    return builder->CreateFPCast(loaded, ToLLVMType(cs, dest));
  } else if (orig->is_bool() && dest->is_int()) { /// bool to int
    return builder->CreateZExtOrTrunc(loaded, ToLLVMType(cs, dest));
  } else if (dest->is_bool()) { /// all types to bool, equivalent to val != 0
    if (orig->is_float()) {
      return builder->CreateFCmpONE(loaded, ConstantFP::get(builder->getFloatTy(), 0.0f));
    } else if (orig->is_pointer()) {
      size_t s1 = cs->get_ptr_size();
      loaded = builder->CreateIntToPtr(loaded, builder->getIntNTy((unsigned) s1));
      return builder->CreateICmpNE(loaded, ConstantInt::get(builder->getIntNTy((unsigned) s1), 0, false));
    } else if (orig->is_int()) {
      return builder->CreateICmpNE(loaded,
          ConstantInt::get(builder->getIntNTy((unsigned) ast_must_cast<PrimitiveType>(orig)->get_size_bits()),
              0,
              false));
    }
  } else if (orig->is_array() && dest->is_array()) {
    // FIXME: casting array of float to/from array of integer is broken
    TAN_ASSERT(false);
  } else if (orig->is_string() && dest->is_pointer()) { /// string to pointer, don't need to do anything
  }

  Error err(cs->_filename, cs->get_source_manager()->get_token(expr->loc()), "Cannot perform type conversion");
  err.raise();
}

DISubroutineType *TypeSystem::CreateFunctionDIType(CompilerSession *cs, Metadata *ret, vector<Metadata *> args) {
  vector<Metadata *> types{ret};
  types.reserve(args.size());
  types.insert(types.begin() + 1, args.begin(), args.end());
  //  return cs->_di_builder
  //    ->createSubroutineType(cs->_di_builder->getOrCreateTypeArray(types), DINode::FlagZero, llvm::dwarf::DW_CC_normal);
  return cs->_di_builder->createSubroutineType(cs->_di_builder->getOrCreateTypeArray(types));
}

/*
void TypeSystem::ResolveTy(ASTContext *ctx, Type *const &p) {
  Ty base = TY_GET_BASE(p->get_ty());
  Ty qual = TY_GET_QUALIFIER(p->get_ty());

  if (p->is_resolved()) { return; }

  /// resolve children
  for (auto *t: p->get_sub_types()) {
    TypeSystem::ResolveTy(ctx, t);
  }

  Token *token = ctx->get_source_manager()->get_token(p->loc());
  auto *tm = Compiler::GetDefaultTargetMachine();
  switch (base) {
    case Ty::STRING:
      p->set_type_name("i8*");
      p->set_size_bits(tm->getPointerSizeInBits(0));
      p->set_align_bits(8);
      break;
    case Ty::ARRAY: {
    }
}
*/

llvm::Type *TypeSystem::ToLLVMType(CompilerSession *cs, Type *p) {
  TAN_ASSERT(p);
  p = p->get_canonical();

  auto *builder = cs->_builder;
  llvm::Type *ret = nullptr;

  if (p->is_primitive()) { /// primitive types
    int size_bits = ast_must_cast<PrimitiveType>(p)->get_size_bits();
    if (p->is_int()) {
      ret = builder->getIntNTy((unsigned) size_bits);
    } else if (p->is_char()) {
      ret = builder->getInt8Ty();
    } else if (p->is_bool()) {
      ret = builder->getInt1Ty();
    } else if (p->is_float()) {
      if (32 == size_bits) {
        ret = builder->getFloatTy();
      } else if (64 == size_bits) {
        ret = builder->getDoubleTy();
      } else {
        TAN_ASSERT(false);
      }
    } else if (p->is_void()) {
      ret = builder->getVoidTy();
    }
  } else if (p->is_string()) { /// str as char*
    ret = builder->getInt8PtrTy();
  } else if (p->is_enum()) { /// enums
    // TODO IMPORTANT: ret = TypeSystem::ToLLVMType(cs, p->get_sub_types()[0]);
    TAN_ASSERT(false);
  } else if (p->is_struct()) { /// struct
    /* TODO IMPORTANT
    vector<Type *> elements{};
    size_t n = p->get_sub_types().size();
    elements.reserve(n);
    for (size_t i = 0; i < n; ++i) {
      elements.push_back(TypeSystem::ToLLVMType(cs, p->get_sub_types()[i]));
    }
    ret = StructType::create(elements, p->get_type_name());
    */
    TAN_ASSERT(false);
  } else if (p->is_array()) { /// array as pointer
    auto *e_type = TypeSystem::ToLLVMType(cs, ast_must_cast<ArrayType>(p)->get_element_type());
    ret = e_type->getPointerTo();
  } else if (p->is_pointer()) { /// pointer
    auto *e_type = TypeSystem::ToLLVMType(cs, ast_must_cast<PointerType>(p)->get_pointee());
    ret = e_type->getPointerTo();
  } else {
    TAN_ASSERT(false);
  }

  return ret;
}

Metadata *TypeSystem::ToLLVMMeta(CompilerSession *cs, Type *p) {
  TAN_ASSERT(p);
  p = p->get_canonical();

  DIType *ret = nullptr;
  auto *tm = Compiler::GetDefaultTargetMachine();

  if (p->is_primitive()) { /// primitive types
    unsigned dwarf_encoding = 0;
    auto *pp = ast_must_cast<PrimitiveType>(p);
    int size_bits = pp->get_size_bits();
    if (pp->is_int()) {
      if (pp->is_unsigned()) {
        if (size_bits == 8) {
          dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
        } else {
          dwarf_encoding = llvm::dwarf::DW_ATE_unsigned;
        }
      } else {
        if (size_bits == 8) {
          dwarf_encoding = llvm::dwarf::DW_ATE_signed_char;
        } else {
          dwarf_encoding = llvm::dwarf::DW_ATE_signed;
        }
      }
    } else if (p->is_char()) {
      dwarf_encoding = llvm::dwarf::DW_ATE_signed_char;
    } else if (p->is_bool()) {
      dwarf_encoding = llvm::dwarf::DW_ATE_boolean;
    } else if (p->is_float()) {
      dwarf_encoding = llvm::dwarf::DW_ATE_float;
    } else if (p->is_void()) {
      dwarf_encoding = llvm::dwarf::DW_ATE_signed;
    }

    ret = cs->_di_builder->createBasicType(p->get_typename(), (uint64_t) size_bits, dwarf_encoding);
  } else if (p->is_string()) { /// str as char*
    auto *e_di_type = cs->_di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
    ret = cs->_di_builder
        ->createPointerType(e_di_type,
            tm->getPointerSizeInBits(0),
            (unsigned) tm->getPointerSizeInBits(0),
            llvm::None,
            p->get_typename());
  } else if (p->is_enum()) { /// enums
    // TODO IMPORTANT
  } else if (p->is_struct()) { /// struct
    /* TODO IMPORTANT
      DIFile *di_file = cs->get_di_file();
      size_t n = p->get_sub_types().size();
      vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        elements.push_back(TypeSystem::ToLLVMMeta(cs, p->get_sub_types()[i]));
      }
      ret = cs->_di_builder
          ->createStructType(cs->get_current_di_scope(),
              p->get_type_name(),
              di_file,
              (unsigned) cs->get_source_manager()->get_line(p->loc()),
              p->get_size_bits(),
              (unsigned) p->get_align_bits(),
              DINode::DIFlags::FlagZero,
              nullptr,
              cs->_di_builder->getOrCreateArray(elements),
              0,
              nullptr,
              p->get_type_name());
    */
    TAN_ASSERT(false);
  } else if (p->is_array()) { /// array as pointer
    auto *sub = TypeSystem::ToLLVMMeta(cs, ast_must_cast<ArrayType>(p)->get_element_type());
    ret = cs->_di_builder
        ->createPointerType((DIType *) sub,
            tm->getPointerSizeInBits(0),
            (unsigned) tm->getPointerSizeInBits(0),
            llvm::None,
            p->get_typename());
  } else if (p->is_pointer()) { /// pointer
    auto *sub = TypeSystem::ToLLVMMeta(cs, ast_must_cast<PointerType>(p)->get_pointee());
    ret = cs->_di_builder
        ->createPointerType((DIType *) sub,
            tm->getPointerSizeInBits(0),
            (unsigned) tm->getPointerSizeInBits(0),
            llvm::None,
            p->get_typename());
  } else {
    TAN_ASSERT(false);
  }

  return ret;
}

Value *TypeSystem::LoadIfLValue(CompilerSession *cs, Expr *expr) {
  Value *val = expr->_llvm_value;
  TAN_ASSERT(val);
  if (expr->is_lvalue()) { return cs->_builder->CreateLoad(val); }
  return val;
}

bool TypeSystem::CanImplicitlyConvert(Type *from, Type *to) {
  // TODO: implicit cast
  return from == to;
}
