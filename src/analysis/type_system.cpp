#include "src/analysis/type_system.h"
#include "src/common.h"
#include "src/ast/constructor.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_context.h"
#include "src/llvm_include.h"
#include <fmt/core.h>

using namespace tanlang;

Value *TypeSystem::ConvertTo(CompilerSession *cs, Value *val, ASTType *orig, ASTType *dest) {
  auto *builder = cs->_builder;

  /// load if lvalue
  Value *loaded = TypeSystem::LoadIfLValue(cs, val, orig);

  bool is_pointer1 = orig->is_ptr();
  bool is_pointer2 = dest->is_ptr();
  size_t s1 = orig->get_size_bits();

  /**
   * NOTE: check enum before checking int
   * */

  /// early return if types are the same
  if (orig == dest || *orig == *dest) { return loaded; };
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return builder->CreateBitCast(loaded, ToLLVMType(cs, dest));
  } else if ((orig->is_enum() && dest->is_int()) || (dest->is_enum() && orig->is_int())) {
    return builder->CreateZExtOrTrunc(loaded, ToLLVMType(cs, dest));
  } else if (orig->is_int() && dest->is_int()) {
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
    return builder->CreateZExtOrTrunc(val, ToLLVMType(cs, dest));
  } else if (dest->is_bool()) { /// all types to bool, equivalent to val != 0
    if (orig->is_float()) {
      return builder->CreateFCmpONE(loaded, ConstantFP::get(builder->getFloatTy(), 0.0f));
    } else if (orig->is_ptr()) {
      s1 = cs->get_ptr_size();
      loaded = builder->CreateIntToPtr(loaded, builder->getIntNTy((unsigned) s1));
      return builder->CreateICmpNE(loaded, ConstantInt::get(builder->getIntNTy((unsigned) s1), 0, false));
    } else {
      return builder->CreateICmpNE(loaded, ConstantInt::get(builder->getIntNTy((unsigned) s1), 0, false));
    }
  } else if (orig->is_array() && dest->is_array()) {
    // FIXME: casting array of float to/from array of integer is broken
    TAN_ASSERT(false);
  }

  /// This shouldn't be executed, since type analysis should have already covered all cases
  TAN_ASSERT(false);
  return nullptr;
}

DISubroutineType *TypeSystem::CreateFunctionDIType(CompilerSession *cs, Metadata *ret, vector<Metadata *> args) {
  vector<Metadata *> types{ret};
  types.reserve(args.size());
  types.insert(types.begin() + 1, args.begin(), args.end());
  //  return cs->_di_builder
  //    ->createSubroutineType(cs->_di_builder->getOrCreateTypeArray(types), DINode::FlagZero, llvm::dwarf::DW_CC_normal);
  return cs->_di_builder->createSubroutineType(cs->_di_builder->getOrCreateTypeArray(types));
}

void TypeSystem::ResolveTy(ASTContext *ctx, ASTType *const &p) {
  Ty base = TY_GET_BASE(p->get_ty());
  Ty qual = TY_GET_QUALIFIER(p->get_ty());

  if (p->is_resolved()) { return; }

  /// resolve children
  for (auto *t: p->get_sub_types()) {
    TypeSystem::ResolveTy(ctx, t);
  }

  Token *token = ctx->get_source_manager()->get_token(p->get_loc());
  auto *tm = Compiler::GetDefaultTargetMachine();
  switch (base) {
    case Ty::INT: {
      if (!p->get_size_bits()) { /// set bit size if not
        p->set_size_bits(ASTType::type_bit_size.at(ctx->get_source_manager()->get_token_str(p->get_loc())));
      }
      p->set_is_int(true);
      p->set_type_name((p->is_unsigned() ? "u" : "i") + std::to_string(p->get_size_bits()));

      /// dwarf encoding
      if (TY_IS(qual, Ty::UNSIGNED)) {
        p->set_is_unsigned(true);
        if (p->get_size_bits() == 8) {
          p->set_dwarf_encoding(llvm::dwarf::DW_ATE_unsigned_char);
        } else {
          p->set_dwarf_encoding(llvm::dwarf::DW_ATE_unsigned);
        }
      } else {
        if (p->get_size_bits() == 8) {
          p->set_dwarf_encoding(llvm::dwarf::DW_ATE_signed_char);
        } else {
          p->set_dwarf_encoding(llvm::dwarf::DW_ATE_signed);
        }
      }
      break;
    }
    case Ty::CHAR:
      p->set_type_name("char");
      p->set_size_bits(8);
      p->set_dwarf_encoding(llvm::dwarf::DW_ATE_unsigned_char);
      p->set_is_unsigned(true);
      p->set_is_int(true);
      break;
    case Ty::BOOL:
      p->set_type_name("bool");
      p->set_size_bits(1);
      p->set_dwarf_encoding(llvm::dwarf::DW_ATE_boolean);
      p->set_is_bool(true);
      break;
    case Ty::FLOAT:
      if (!p->get_size_bits()) { /// set bit size if not
        p->set_size_bits(ASTType::type_bit_size.at(ctx->get_source_manager()->get_token_str(p->get_loc())));
      }
      p->set_type_name("f" + std::to_string(p->get_size_bits()));
      p->set_dwarf_encoding(llvm::dwarf::DW_ATE_float);
      p->set_is_float(true);
      break;
    case Ty::STRING:
      p->set_type_name("u8*");
      p->set_size_bits(tm->getPointerSizeInBits(0));
      p->set_align_bits(8);
      break;
    case Ty::VOID:
      p->set_type_name("void");
      p->set_size_bits(0);
      p->set_dwarf_encoding(llvm::dwarf::DW_ATE_signed);
      break;
    case Ty::ENUM: {
      /// underlying type is i32
      auto sub = ASTType::GetI32Type(ctx, p->get_loc());
      p->set_sub_types({sub});
      p->set_size_bits(sub->get_size_bits());
      p->set_align_bits(sub->get_align_bits());
      p->set_dwarf_encoding(sub->get_dwarf_encoding());
      p->set_is_unsigned(sub->is_unsigned());
      p->set_is_int(sub->is_int());
      p->set_is_enum(true);
      /// _type_name, however, is set during analysis
      TAN_ASSERT(p->get_type_name() != "");
      break;
    }
    case Ty::STRUCT: {
      TAN_ASSERT(p->get_type_name() != "");
      if (p->is_forward_decl()) {
        /// we're not supposed to resolve a forward declaration here, as all forward decls should be replaced
        /// by an actual struct declaration by now
        Error err(ctx->_filename, token, "Unresolved forward declaration of type");
        err.raise();
      }

      /// align size is the max element size, if no element, 8 bits
      /// size is the number of elements * align size
      p->set_align_bits(8);
      auto &sub_types = p->get_sub_types();
      size_t n = sub_types.size();
      for (size_t i = 0; i < n; ++i) {
        auto et = sub_types[i];
        auto s = et->get_size_bits();
        if (s > p->get_align_bits()) { p->set_align_bits(s); }
      }
      p->set_size_bits(n * p->get_align_bits());
      p->set_is_struct(true);
      break;
    }
    case Ty::ARRAY: {
      if (p->get_sub_types().size() == 0) {
        Error err(ctx->_filename, token, "Invalid type");
        err.raise();
      }
      auto et = p->get_sub_types()[0];
      /// typename = "<element type>[<n_elements>]"
      p->set_type_name(fmt::format("{}[{}]", et->get_type_name(), std::to_string(p->get_array_size())));
      p->set_is_array(true);
      p->set_size_bits(tm->getPointerSizeInBits(0));
      p->set_align_bits(et->get_size_bits());
      p->set_dwarf_encoding(llvm::dwarf::DW_ATE_address);
      break;
    }
    case Ty::POINTER: {
      if (p->get_sub_types().size() == 0) {
        Error err(ctx->_filename, token, "Invalid type");
        err.raise();
      }
      auto &e = p->get_sub_types()[0];
      TypeSystem::ResolveTy(ctx, e);
      p->set_type_name(e->get_type_name() + "*");
      p->set_size_bits(tm->getPointerSizeInBits(0));
      p->set_align_bits(e->get_size_bits());
      p->set_dwarf_encoding(llvm::dwarf::DW_ATE_address);
      break;
    }
    case Ty::TYPE_REF: {
      if (!p->get_canonical_type()) {
        Error err(ctx->_filename, token, "Invalid type name");
        err.raise();
      }
      break;
    }
    default:
      Error err(ctx->_filename, token, "Invalid type");
      err.raise();
  }
  p->set_resolved(true);
}

void TypeSystem::SetDefaultConstructor(ASTContext *ctx, ASTType *const &p) {
  TAN_ASSERT(p->is_resolved());
  Ty base = TY_GET_BASE(p->get_ty());

  switch (base) {
    case Ty::INT:
      p->set_constructor(BasicConstructor::CreateIntegerConstructor(ctx,
          p->get_loc(),
          0,
          p->get_size_bits(),
          p->is_unsigned()));
      break;
    case Ty::CHAR:
      p->set_constructor(BasicConstructor::CreateCharConstructor(ctx, p->get_loc()));
      break;
    case Ty::BOOL:
      p->set_constructor(BasicConstructor::CreateBoolConstructor(ctx, p->get_loc()));
      break;
    case Ty::FLOAT:
      p->set_constructor(BasicConstructor::CreateFPConstructor(ctx, p->get_loc(), 0, p->get_size_bits()));
      break;
    case Ty::STRING:
      p->set_constructor(BasicConstructor::CreateStringConstructor(ctx, p->get_loc()));
      break;
    case Ty::ENUM:
      // TODO: default value 0?
      p->set_constructor(BasicConstructor::CreateIntegerConstructor(ctx, p->get_loc(), 0, p->get_size_bits()));
      break;
    case Ty::STRUCT:
      // TODO: p->set_constructor()
      break;
    case Ty::ARRAY: {
      vector<ASTType *> sub_types = p->get_sub_types();
      TAN_ASSERT(!sub_types.empty());
      p->set_constructor(BasicConstructor::CreateArrayConstructor(ctx, p->get_loc(), sub_types[0]));
      break;
    }
    case Ty::POINTER:
      p->set_constructor(BasicConstructor::CreateNullPointerConstructor(ctx, p->get_loc(), p->get_contained_ty()));
      break;
    default:
      // TODO: TAN_ASSERT(false);
      break;
  }
}

Type *TypeSystem::ToLLVMType(CompilerSession *cs, ASTType *p) {
  TAN_ASSERT(p);
  TAN_ASSERT(p = p->get_canonical_type());

  if (p->get_llvm_type()) { return p->get_llvm_type(); } /// avoid creating duplicated types

  TAN_ASSERT(p->is_resolved());

  auto *builder = cs->_builder;
  Ty base = TY_GET_BASE(p->get_ty());
  Type *type = nullptr;
  switch (base) {
    case Ty::INT:
      type = builder->getIntNTy((unsigned) p->get_size_bits());
      break;
    case Ty::CHAR:
      type = builder->getInt8Ty();
      break;
    case Ty::BOOL:
      type = builder->getInt1Ty();
      break;
    case Ty::FLOAT:
      if (32 == p->get_size_bits()) {
        type = builder->getFloatTy();
      } else if (64 == p->get_size_bits()) {
        type = builder->getDoubleTy();
      } else {
        TAN_ASSERT(false);
      }
      break;
    case Ty::STRING:
      type = builder->getInt8PtrTy(); /// str as char*
      break;
    case Ty::VOID:
      type = builder->getVoidTy();
      break;
    case Ty::ENUM:
      type = TypeSystem::ToLLVMType(cs, p->get_sub_types()[0]);
      break;
    case Ty::STRUCT: {
      vector<Type *> elements{};
      size_t n = p->get_sub_types().size();
      elements.reserve(n);
      for (size_t i = 0; i < n; ++i) {
        elements.push_back(TypeSystem::ToLLVMType(cs, p->get_sub_types()[i]));
      }
      type = StructType::create(elements, p->get_type_name());
      break;
    }
    case Ty::ARRAY: /// during analysis phase, array is different from pointer, but during _codegen, they are the same
    case Ty::POINTER: {
      auto e_type = TypeSystem::ToLLVMType(cs, p->get_sub_types()[0]);
      type = e_type->getPointerTo();
      break;
    }
    default:
      TAN_ASSERT(false);
  }

  p->set_llvm_type(type);
  return type;
}

Metadata *TypeSystem::ToLLVMMeta(CompilerSession *cs, ASTType *p) {
  TAN_ASSERT(p->is_resolved());

  Ty base = TY_GET_BASE(p->get_ty());
  // TODO: Ty qual = TY_GET_QUALIFIER(_ty);
  DIType *ret = nullptr;
  switch (base) {
    case Ty::CHAR:
    case Ty::INT:
    case Ty::BOOL:
    case Ty::FLOAT:
    case Ty::VOID:
    case Ty::ENUM:
      ret = cs->_di_builder->createBasicType(p->get_type_name(), p->get_size_bits(), p->get_dwarf_encoding());
      break;
    case Ty::STRING: {
      auto *e_di_type = cs->_di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = cs->_di_builder
          ->createPointerType(e_di_type,
              p->get_size_bits(),
              (unsigned) p->get_align_bits(),
              llvm::None,
              p->get_type_name());
      break;
    }
    case Ty::STRUCT: {
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
              (unsigned) cs->get_source_manager()->get_line(p->get_loc()),
              p->get_size_bits(),
              (unsigned) p->get_align_bits(),
              DINode::DIFlags::FlagZero,
              nullptr,
              cs->_di_builder->getOrCreateArray(elements),
              0,
              nullptr,
              p->get_type_name());
      break;
    }
    case Ty::ARRAY:
    case Ty::POINTER: {
      auto e = p->get_sub_types()[0];
      auto *e_di_type = TypeSystem::ToLLVMMeta(cs, e);
      ret = cs->_di_builder
          ->createPointerType((DIType *) e_di_type,
              p->get_size_bits(),
              (unsigned) p->get_align_bits(),
              llvm::None,
              p->get_type_name());
      break;
    }
    case Ty::TYPE_REF:
      ret = (DIType *) TypeSystem::ToLLVMMeta(cs, p->get_canonical_type());
      break;
    default:
      TAN_ASSERT(false);
  }
  return ret;
}

Value *TypeSystem::LoadIfLValue(CompilerSession *cs, Value *val, ASTType *type) {
  if (type->is_lvalue()) { return cs->_builder->CreateLoad(val); }
  return val;
}
