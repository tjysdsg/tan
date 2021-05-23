#include <iostream>
#include "intrinsic.h"
#include "src/common.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_control_flow.h"
#include "src/ast/factory.h"
#include "src/analysis/analysis.h"
#include "src/analysis/type_system.h"
#include "src/ast/ast_ty.h"
#include "src/ast/ast_func.h"
#include "compiler_session.h"
#include "compiler.h"
#include "token.h"

namespace tanlang {

ASTNodePtr get_id_referred(CompilerSession *cs, const ASTNodePtr &p) {
  return cs->get(p->get_data<str>());
}

/// \section Types

Type *to_llvm_type(CompilerSession *cs, const ASTTyPtr &p) {
  auto *builder = cs->_builder;
  resolve_ty(cs, p);
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
      type = to_llvm_type(cs, get_ty(p->get_child_at(0)));
      break;
    case Ty::STRUCT: {
      auto *struct_type = StructType::create(*cs->get_context(), p->_type_name);
      vector<Type *> body{};
      size_t n = p->get_children_size();
      body.reserve(n);
      for (size_t i = 1; i < n; ++i) {
        body.push_back(to_llvm_type(cs, get_ty(p->get_child_at(i))));
      }
      struct_type->setBody(body);
      type = struct_type;
      break;
    }
    case Ty::ARRAY: /// during analysis phase, array is different from pointer, but during _codegen, they are the same
    case Ty::POINTER: {
      auto e_type = to_llvm_type(cs, get_ty(p->get_child_at(0)));
      type = e_type->getPointerTo();
      break;
    }
    default:
      TAN_ASSERT(false);
  }
  return type;
}

Metadata *to_llvm_meta(CompilerSession *cs, const ASTTyPtr &p) {
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
        elements.push_back(to_llvm_meta(cs, get_ty(e)));
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
      auto e = p->get_child_at<ASTTy>(0);
      auto *e_di_type = to_llvm_meta(cs, e);
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

ASTTyPtr get_ptr_to(CompilerSession *cs, const ASTTyPtr &p) { return create_ty(cs, Ty::POINTER, {p->_ty}, false); }

ASTTyPtr get_contained_ty(CompilerSession *cs, const ASTTyPtr &p) {
  if (p->_tyty == Ty::STRING) {
    return create_ty(cs, Ty::CHAR, vector<ASTNodePtr>(), false);
  } else if (p->_is_ptr) {
    TAN_ASSERT(p->get_children_size());
    auto ret = p->get_child_at<ASTTy>(0);
    TAN_ASSERT(ret);
    return ret;
  } else {
    return nullptr;
  }
}

ASTTyPtr get_struct_member_ty(const ASTTyPtr &p, size_t i) {
  TAN_ASSERT(p->_tyty == Ty::STRUCT);
  return p->_children[i]->_ty;
}

size_t get_struct_member_index(const ASTTyPtr &p, const str &name) {
  auto search = p->_member_indices.find(name);
  if (search == p->_member_indices.end()) {
    return (size_t) (-1);
  }
  return search->second;
}

/// \section Analysis

str get_source_location(CompilerSession *cs, ASTNodePtr p) {
  return cs->_filename + ":" + std::to_string(p->get_line());
}

} // namespace tanlang
