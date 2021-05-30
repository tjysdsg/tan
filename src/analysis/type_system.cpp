#include "src/analysis/type_system.h"
#include "src/common.h"
#include "src/analysis/ast_helper.h"
#include "compiler_session.h"
#include "compiler.h"
#include "src/ast/ast_ty.h"
#include "src/llvm_include.h"
#include <fmt/core.h>

using namespace tanlang;

llvm::Value *TypeSystem::ConvertTo(CompilerSession *cs, llvm::Value *val, ASTTypePtr orig, ASTTypePtr dest) {
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
    return builder->CreateBitCast(loaded, ToLLVMType(cs, dest));
  } else if ((orig->_is_enum && dest->_is_int) || (dest->_is_enum && orig->_is_int)) {
    return builder->CreateZExtOrTrunc(loaded, ToLLVMType(cs, dest));
  } else if (orig->_is_int && dest->_is_int) {
    return builder->CreateZExtOrTrunc(loaded, ToLLVMType(cs, dest));
  } else if (orig->_is_int && dest->_is_float) { /// int to float/double
    if (orig->_is_unsigned) {
      return builder->CreateUIToFP(loaded, ToLLVMType(cs, dest));
    } else {
      return builder->CreateSIToFP(loaded, ToLLVMType(cs, dest));
    }
  } else if (orig->_is_float && dest->_is_int) { /// float/double to int
    if (orig->_is_unsigned) {
      return builder->CreateFPToUI(loaded, ToLLVMType(cs, dest));
    } else {
      return builder->CreateFPToSI(loaded, ToLLVMType(cs, dest));
    }
  } else if (orig->_is_float && dest->_is_float) { /// float <-> double
    return builder->CreateFPCast(loaded, ToLLVMType(cs, dest));
  } else if (orig->_is_bool && dest->_is_int) { /// bool to int
    return builder->CreateZExtOrTrunc(val, ToLLVMType(cs, dest));
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

int TypeSystem::CanImplicitCast(CompilerSession *cs, ASTTypePtr t1, ASTTypePtr t2) {
  ASTHelper h(cs);
  TAN_ASSERT(t1);
  TAN_ASSERT(t2);
  if (t1.get() == t2.get()) { return 0; } /// since some ASTType are cached, compare pointer and early return
  if (*t1 == *t2) { return 0; }
  size_t s1 = t1->_size_bits;
  size_t s2 = t2->_size_bits;

  // TODO: support implicit cast of different pointer types
  if (t1->_is_ptr && t2->_is_ptr && *h.get_contained_ty(t1) == *h.get_contained_ty(t2)) {
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
  } else if (t1->_is_array && t2->_is_array) { /// arrays, FIXME: move this to before the _is_ptr check
    /// array size must be the same
    if (t1->_array_size != t2->_array_size) { return -1; }
    /// the element type can be implicitly casted as long as the elements have the same size
    if (h.get_contained_ty(t1)->_size_bits != h.get_contained_ty(t2)->_size_bits) { return -1; }
    return CanImplicitCast(cs, h.get_contained_ty(t1), h.get_contained_ty(t2));
  }
  return -1;
}

void TypeSystem::ResolveTy(CompilerSession *cs, ASTTypePtr p) {
  Ty base = TY_GET_BASE(p->_tyty);
  Ty qual = TY_GET_QUALIFIER(p->_tyty);
  if (p->_resolved) {
    if (base == Ty::STRUCT) {
      if (!p->_is_forward_decl) { return; }
    } else { return; }
  }
  /// resolve_ty children if they are ASTType
  for (const auto &c: p->get_children()) {
    auto t = ast_cast<ASTType>(c);
    if (t && t->get_node_type() == ASTNodeType::TY && !t->_resolved) {
      TypeSystem::ResolveTy(cs, t);
    }
  }
  auto *tm = Compiler::GetDefaultTargetMachine();
  switch (base) {
    case Ty::INT: {
      p->_size_bits = 32;
      p->_type_name = "i32";
      p->_is_int = true;
      p->_default_value.emplace<uint64_t>(0);
      if (TY_IS(qual, Ty::BIT8)) {
        p->_size_bits = 8;
        p->_type_name = "i8";
      } else if (TY_IS(qual, Ty::BIT16)) {
        p->_size_bits = 16;
        p->_type_name = "i16";
      } else if (TY_IS(qual, Ty::BIT64)) {
        p->_size_bits = 64;
        p->_type_name = "i64";
      }
      if (TY_IS(qual, Ty::UNSIGNED)) {
        p->_is_unsigned = true;
        if (p->_size_bits == 8) {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
        } else {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned;
        }
      } else {
        if (p->_size_bits == 8) {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed_char;
        } else {
          p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed;
        }
      }
      break;
    }
    case Ty::CHAR:
      p->_type_name = "char";
      p->_size_bits = 8;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_unsigned_char;
      p->_is_unsigned = true;
      p->_default_value.emplace<uint64_t>(0);
      p->_is_int = true;
      break;
    case Ty::BOOL:
      p->_type_name = "bool";
      p->_size_bits = 1;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_boolean;
      p->_default_value.emplace<uint64_t>(0);
      p->_is_bool = true;
      break;
    case Ty::FLOAT:
      p->_type_name = "float";
      p->_size_bits = 32;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_float;
      p->_default_value.emplace<float>(0);
      p->_is_float = true;
      break;
    case Ty::DOUBLE:
      p->_type_name = "double";
      p->_size_bits = 64;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_float;
      p->_default_value.emplace<double>(0);
      p->_is_float = true;
      break;
    case Ty::STRING:
      p->_type_name = "u8*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_default_value.emplace<str>("");
      p->_align_bits = 8;
      p->_is_ptr = true;
      break;
    case Ty::VOID:
      p->_type_name = "void";
      p->_size_bits = 0;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_signed;
      break;
    case Ty::ENUM: {
      auto sub = p->get_child_at<ASTType>(0);
      TAN_ASSERT(sub);
      p->_size_bits = sub->_size_bits;
      p->_align_bits = sub->_align_bits;
      p->_dwarf_encoding = sub->_dwarf_encoding;
      p->_default_value = sub->_default_value;
      p->_is_unsigned = sub->_is_unsigned;
      p->_is_int = sub->_is_int;
      p->_is_enum = true;
      /// _type_name, however, is set by ASTEnum::nud
      break;
    }
    case Ty::STRUCT: {
      /// align size is the max element size, if no element, 8 bits
      /// size is the number of elements * align size
      if (p->_is_forward_decl) {
        auto real = cs->get_type(p->_type_name);
        if (!real) {
          report_error(cs->_filename, p->get_token(), "Incomplete type");
        }
        *p = *real;
        p->_is_forward_decl = false;
      } else {
        p->_align_bits = 8;
        size_t n = p->get_children_size();
        for (size_t i = 0; i < n; ++i) {
          auto et = p->get_child_at<ASTType>(i);
          auto s = et->_size_bits;
          if (s > p->_align_bits) { p->_align_bits = s; }
        }
        p->_size_bits = n * p->_align_bits;
        p->_is_struct = true;
      }
      break;
    }
    case Ty::ARRAY: {
      if (p->get_children_size() == 0) {
        report_error(cs->_filename, p->get_token(), "Invalid type");
      }
      auto et = p->get_child_at<ASTType>(0);
      /// typename = "<element type>[<n_elements>]"
      p->_type_name = fmt::format("{}[{}]", et->_type_name, std::to_string(p->_array_size));
      p->_is_ptr = true; /// FIXME: remove _is_ptr field
      p->_is_array = true;
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = et->_size_bits;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    case Ty::POINTER: {
      if (p->get_children_size() == 0) {
        report_error(cs->_filename, p->get_token(), "Invalid type");
      }
      auto e = p->get_child_at<ASTType>(0);
      TypeSystem::ResolveTy(cs, e);
      p->_type_name = e->_type_name + "*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = e->_size_bits;
      p->_is_ptr = true;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    default:
      report_error(cs->_filename, p->get_token(), "Invalid type");
  }
  p->_resolved = true;
}

Type *TypeSystem::ToLLVMType(CompilerSession *cs, const ASTTypePtr &p) {
  TAN_ASSERT(p->_resolved);

  ASTHelper h(cs);
  auto *builder = cs->_builder;
  Ty base = TY_GET_BASE(p->_tyty);
  llvm::Type *type = nullptr;
  switch (base) {
    case Ty::INT:
      type = builder->getIntNTy((unsigned) p->_size_bits);
      break;
    case Ty::CHAR:
      type = builder->getInt8Ty();
      break;
    case Ty::BOOL:
      type = builder->getInt1Ty();
      break;
    case Ty::FLOAT:
      type = builder->getFloatTy();
      break;
    case Ty::DOUBLE:
      type = builder->getDoubleTy();
      break;
    case Ty::STRING:
      type = builder->getInt8PtrTy(); /// str as char*
      break;
    case Ty::VOID:
      type = builder->getVoidTy();
      break;
    case Ty::ENUM:
      type = ToLLVMType(cs, h.get_ty(p->get_child_at(0)));
      break;
    case Ty::STRUCT: {
      auto *struct_type = StructType::create(*cs->get_context(), p->_type_name);
      vector<Type *> body{};
      size_t n = p->get_children_size();
      body.reserve(n);
      for (size_t i = 1; i < n; ++i) {
        body.push_back(ToLLVMType(cs, h.get_ty(p->get_child_at(i))));
      }
      struct_type->setBody(body);
      type = struct_type;
      break;
    }
    case Ty::ARRAY: /// during analysis phase, array is different from pointer, but during _codegen, they are the same
    case Ty::POINTER: {
      auto e_type = ToLLVMType(cs, h.get_ty(p->get_child_at(0)));
      type = e_type->getPointerTo();
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return type;
}

Metadata *TypeSystem::ToLLVMMeta(CompilerSession *cs, const ASTTypePtr &p) {
  TAN_ASSERT(p->_resolved);

  ASTHelper h(cs);
  Ty base = TY_GET_BASE(p->_tyty);
  // TODO: Ty qual = TY_GET_QUALIFIER(_tyty);
  DIType *ret = nullptr;
  switch (base) {
    case Ty::CHAR:
    case Ty::INT:
    case Ty::BOOL:
    case Ty::FLOAT:
    case Ty::VOID:
    case Ty::DOUBLE:
    case Ty::ENUM:
      ret = cs->_di_builder->createBasicType(p->_type_name, p->_size_bits, p->_dwarf_encoding);
      break;
    case Ty::STRING: {
      auto *e_di_type = cs->_di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = cs->_di_builder
          ->createPointerType(e_di_type, p->_size_bits, (unsigned) p->_align_bits, llvm::None, p->_type_name);
      break;
    }
    case Ty::STRUCT: {
      DIFile *di_file = cs->get_di_file();
      size_t n = p->get_children_size();
      vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        auto e = p->get_child_at(i); // ASTVarDecl
        elements.push_back(ToLLVMMeta(cs, h.get_ty(e)));
      }
      ret = cs->_di_builder
          ->createStructType(cs->get_current_di_scope(),
              p->_type_name,
              di_file,
              (unsigned) p->get_line(),
              p->_size_bits,
              (unsigned) p->_align_bits,
              DINode::DIFlags::FlagZero,
              nullptr,
              cs->_di_builder->getOrCreateArray(elements),
              0,
              nullptr,
              p->_type_name);
      break;
    }
    case Ty::ARRAY:
    case Ty::POINTER: {
      auto e = p->get_child_at<ASTType>(0);
      auto *e_di_type = ToLLVMMeta(cs, e);
      ret = cs->_di_builder
          ->createPointerType((DIType *) e_di_type,
              p->_size_bits,
              (unsigned) p->_align_bits,
              llvm::None,
              p->_type_name);
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return ret;
}
