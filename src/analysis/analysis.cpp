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

ASTNodePtr get_id_referred(CompilerSession *cs, const ASTNodePtr &p) { return cs->get(p->_name); }

/// \section General

size_t get_n_children(const ASTNodePtr &p) { return p->_children.size(); }

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
      type = to_llvm_type(cs, p->_children[0]->_ty);
      break;
    case Ty::STRUCT: {
      auto *struct_type = StructType::create(*cs->get_context(), p->_type_name);
      vector<Type *> body{};
      size_t n = p->_children.size();
      body.reserve(n);
      for (size_t i = 1; i < n; ++i) { body.push_back(to_llvm_type(cs, p->_children[i]->_ty)); }
      struct_type->setBody(body);
      type = struct_type;
      break;
    }
    case Ty::ARRAY: /// during analysis phase, array is different from pointer, but during _codegen, they are the same
    case Ty::POINTER: {
      auto e_type = to_llvm_type(cs, p->_children[0]->_ty);
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
      size_t n = p->_children.size();
      vector<Metadata *> elements(n);
      for (size_t i = 1; i < n; ++i) {
        auto e = p->_children[i]; // ASTVarDecl
        elements.push_back(to_llvm_meta(cs, e->_ty));
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
      auto e = ast_cast<ASTTy>(p->_children[0]);
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

str get_type_name(const ASTNodePtr &p) { return p->_ty->_type_name; }

void resolve_ty(CompilerSession *cs, const ASTTyPtr &p) {
  Ty base = TY_GET_BASE(p->_tyty);
  Ty qual = TY_GET_QUALIFIER(p->_tyty);
  if (p->_resolved) {
    if (base == Ty::STRUCT) {
      if (!p->_is_forward_decl) { return; }
    } else { return; }
  }
  p->_ty = p;
  /// resolve_ty children if they are ASTTy
  for (const auto &c: p->_children) {
    auto t = ast_cast<ASTTy>(c);
    if (t && t->_type == ASTType::TY && !t->_resolved) { resolve_ty(cs, t); }
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
      auto sub = ast_cast<ASTTy>(p->_children[0]);
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
        auto real = ast_cast<ASTTy>(cs->get(p->_type_name));
        if (!real) { report_error(cs, p, "Incomplete type"); }
        *p = *real;
        p->_is_forward_decl = false;
      } else {
        p->_align_bits = 8;
        size_t n = p->_children.size();
        for (size_t i = 0; i < n; ++i) {
          auto et = ast_cast<ASTTy>(p->_children[i]);
          auto s = et->_size_bits;
          if (s > p->_align_bits) { p->_align_bits = s; }
        }
        p->_size_bits = n * p->_align_bits;
        p->_is_struct = true;
      }
      break;
    }
    case Ty::ARRAY: {
      if (p->_children.empty()) { report_error(cs, p, "Invalid type"); }
      auto et = ast_cast<ASTTy>(p->_children[0]);
      TAN_ASSERT(et);
      p->_type_name = "[" + get_type_name(et) + ", " + std::to_string(p->_children.size()) + "]";
      p->_is_ptr = true;
      p->_is_array = true;
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = et->_size_bits;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    case Ty::POINTER: {
      if (p->_children.empty()) { report_error(cs, p, "Invalid type"); }
      auto e = ast_cast<ASTTy>(p->_children[0]);
      TAN_ASSERT(e);
      resolve_ty(cs, e);
      p->_type_name = get_type_name(e) + "*";
      p->_size_bits = tm->getPointerSizeInBits(0);
      p->_align_bits = e->_size_bits;
      p->_is_ptr = true;
      p->_dwarf_encoding = llvm::dwarf::DW_ATE_address;
      break;
    }
    default:
      report_error(cs, p, "Invalid type");
  }
  p->_resolved = true;
}

ASTTyPtr get_ptr_to(CompilerSession *cs, const ASTTyPtr &p) { return create_ty(cs, Ty::POINTER, {p->_ty}, false); }

bool is_lvalue(const ASTNodePtr &p) { return p->_ty->_is_lvalue; }

ASTTyPtr get_contained_ty(CompilerSession *cs, const ASTTyPtr &p) {
  if (p->_tyty == Ty::STRING) { return create_ty(cs, Ty::CHAR, vector<ASTNodePtr>(), false); }
  else if (p->_is_ptr) {
    TAN_ASSERT(p->_children.size());
    auto ret = ast_cast<ASTTy>(p->_children[0]);
    TAN_ASSERT(ret);
    return ret;
  } else { return nullptr; }
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

static void analyze_intrinsic(CompilerSession *cs, ASTNodePtr p) {
  auto pi = ast_cast<Intrinsic>(p);
  TAN_ASSERT(pi);
  TAN_ASSERT(!p->_children.empty());

  auto c = p->_children[0];
  TAN_ASSERT(c->_is_named);  // both function call and identifier have a name
  auto void_type = create_ty(cs, Ty::VOID);
  auto q = Intrinsic::intrinsics.find(c->_name);
  if (q == Intrinsic::intrinsics.end()) { report_error(cs, p, "Invalid intrinsic"); }
  pi->_intrinsic_type = q->second;
  switch (pi->_intrinsic_type) {
    case IntrinsicType::STACK_TRACE:
    case IntrinsicType::ABORT:
    case IntrinsicType::NOOP: {
      p->_ty = void_type;
      break;
    }
    case IntrinsicType::LINENO: {
      p->_children[0] = ast_create_numeric_literal(cs, c->get_line());
      break;
    }
    case IntrinsicType::FILENAME: {
      p->_children[0] = ast_create_string_literal(cs, cs->_filename);
      break;
    }
    case IntrinsicType::GET_DECL: {
      p->_ty = create_ty(cs, Ty::STRING);
      TAN_ASSERT(c->_type == ASTType::STRING_LITERAL);
      // TODO: set p->_value to the source code of p
      break;
    }
    case IntrinsicType::COMP_PRINT: {
      p->_ty = void_type;
      if (c->_type != ASTType::STRING_LITERAL) {
        report_error(cs, p, "Invalid call to compprint, one argument with type 'str' required");
      }
      std::cout << "Message (" << get_source_location(cs, p) << "): " << std::get<str>(c->_value) << "\n";
      break;
    }
    default:
      report_error(cs, p, "Unknown intrinsic");
  }
}

void analyze(CompilerSession *cs, const ASTNodePtr &p) {
  p->_scope = cs->get_current_scope();
  // TODO: update _cs->_current_token

  if (p->_type == ASTType::FUNC_DECL) { /// children will not be automatically parsed for function declaration
    for (const auto &sub: p->_children) { analyze(cs, sub); }
  }

  switch (p->_type) {
    /////////////////////////// binary ops ///////////////////////////////////
    // TODO: create a new ASTType for unary plus and minus
    case ASTType::SUM:
    case ASTType::SUBTRACT: {
      /// unary plus/minus
      if (p->_children.size() == 1) {
        p->_ty = ast_cast<ASTTy>(p->_children[0]);
        break;
      }
    }
      // fallthrough
    case ASTType::MULTIPLY:
    case ASTType::DIVIDE:
    case ASTType::MOD: {
      int i = TypeSystem::CanImplicitCast(cs, p->_children[0]->_ty, p->_children[1]->_ty);
      if (i == -1) { report_error(cs, p, "Cannot perform implicit type conversion"); }
      p->_ty = ast_cast<ASTTy>(p->_children[(size_t) i]);
      p->_dominant_idx = (size_t) i;
      break;
    }
    case ASTType::GT:
    case ASTType::GE:
    case ASTType::LT:
    case ASTType::LE:
    case ASTType::EQ:
    case ASTType::NE:
      p->_ty = create_ty(cs, Ty::BOOL);
      break;
    case ASTType::ASSIGN: {
      p->_ty = p->_children[0]->_ty;
      if (TypeSystem::CanImplicitCast(cs, p->_ty, p->_children[1]->_ty) != 0) {
        report_error(cs, p, "Cannot perform implicit type conversion");
      }
      break;
    }
    case ASTType::CAST: {
      p->_ty = make_ptr<ASTTy>(*p->_children[1]->_ty);
      p->_ty->_is_lvalue = p->_children[0]->_ty->_is_lvalue;
      if (TypeSystem::CanImplicitCast(cs, p->_ty, p->_children[0]->_ty) != 0) {
        report_error(cs, p, "Cannot perform implicit type conversion");
      }
      break;
    }
    case ASTType::MEMBER_ACCESS: {
      auto lhs = p->_children[0];
      auto pma = ast_cast<ASTMemberAccess>(p);
      TAN_ASSERT(pma);

      if (pma->_access_type == MemberAccessType::MemberAccessDeref) { /// pointer dereference
        auto ty = lhs->_ty;
        TAN_ASSERT(ty->_is_ptr);
        ty = std::make_shared<ASTTy>(*get_contained_ty(cs, ty));
        ty->_is_lvalue = true;
        p->_ty = ty;
      } else if (pma->_access_type == MemberAccessType::MemberAccessBracket) {
        auto rhs = p->_children[1];
        ASTTyPtr ty = lhs->_ty;
        if (!ty->_is_ptr) { report_error(cs, p, "Expect a pointer or an array"); }
        ty = std::make_shared<ASTTy>(*get_contained_ty(cs, ty));
        ty->_is_lvalue = true;
        if (!ty) { report_error(cs, p, "Unable to perform bracket access"); }
        p->_ty = ty;
        if (rhs->_type == ASTType::NUM_LITERAL) {
          if (!rhs->_ty->_is_int) { report_error(cs, p, "Expect an integer specifying array size"); }
          auto size = std::get<uint64_t>(rhs->_value); // underflow
          if (rhs->_ty->_is_array && size >= lhs->_ty->_array_size) {
            report_error(cs,
                p,
                "Index " + std::to_string(size) + " out of bound, the array size is "
                    + std::to_string(lhs->_ty->_array_size));
          }
        }
      } else if (p->_children[1]->_type == ASTType::ID) { /// member variable or enum
        auto rhs = p->_children[1];
        if (lhs->_ty->_is_enum) {
          // TODO: Member access of enums
        } else {
          pma->_access_type = MemberAccessType::MemberAccessMemberVariable;
          if (!lhs->_ty->_is_lvalue && !lhs->_ty->_is_ptr) { report_error(cs, p, "Invalid left-hand operand"); }
          str m_name = rhs->_name;
          std::shared_ptr<ASTTy> struct_ast = nullptr;
          /// auto dereference pointers
          if (lhs->_ty->_is_ptr) {
            struct_ast = ast_cast<ASTTy>(cs->get(get_contained_ty(cs, lhs->_ty)->_type_name));
          } else {
            struct_ast = ast_cast<ASTTy>(cs->get(lhs->_ty->_type_name));
          }
          TAN_ASSERT(struct_ast);
          pma->_access_idx = get_struct_member_index(struct_ast, m_name);
          p->_ty = make_ptr<ASTTy>(*get_struct_member_ty(struct_ast, pma->_access_idx));
          p->_ty->_is_lvalue = true;
        }
      } else if (pma->_access_type == MemberAccessType::MemberAccessMemberFunction) { /// method call
        auto rhs = p->_children[1];
        if (!lhs->_ty->_is_lvalue && !lhs->_ty->_is_ptr) {
          report_error(cs, p, "Method calls require left-hand operand to be an lvalue or a pointer");
        }
        /// get address of the struct instance
        if (lhs->_ty->_is_lvalue && !lhs->_ty->_is_ptr) {
          auto tmp = ast_create_address_of(cs, lhs);
          analyze(cs, tmp);
          rhs->_children.insert(rhs->_children.begin(), tmp);
        } else {
          rhs->_children.insert(rhs->_children.begin(), lhs);
        }
        /// TODO: postpone analysis of FUNC_CALL until now
        analyze(cs, rhs);
        p->_ty = rhs->_ty;
      } else { report_error(cs, p, "Invalid right-hand operand"); }
      break;
    }
      /////////////////////////// unary ops ////////////////////////////////////
    case ASTType::RET:
      // TODO: check if return type can be implicitly cast to function return type
      break;
    case ASTType::LNOT:
      p->_ty = create_ty(cs, Ty::BOOL);
      break;
    case ASTType::BNOT:
      p->_ty = p->_children[0]->_ty;
      break;
    case ASTType::ADDRESS_OF: {
      if (!(p->_ty = p->_children[0]->_ty)) { report_error(cs, p, "Invalid operand"); }
      p->_ty = get_ptr_to(cs, p->_ty);
      break;
    }
    case ASTType::ID: {
      auto referred = get_id_referred(cs, p);
      p->_children.push_back(referred);
      p->_ty = referred->_ty;
      break;
    }
      //////////////////////// literals ///////////////////////////////////////
    case ASTType::CHAR_LITERAL: {
      p->_ty = create_ty(cs, Ty::CHAR, {});
      p->_value = static_cast<uint64_t>(p->get_token()->value[0]);
      p->_ty->_default_value = std::get<uint64_t>(p->_value);
      break;
    }
    case ASTType::NUM_LITERAL: {
      if (p->get_token()->type == TokenType::INT) {
        auto tyty = Ty::INT;
        if (p->get_token()->is_unsigned) { tyty = TY_OR(tyty, Ty::UNSIGNED); }
        p->_ty = create_ty(cs, tyty);
      } else if (p->get_token()->type == TokenType::FLOAT) {
        p->_ty = create_ty(cs, Ty::FLOAT);
      }
      break;
    }
    case ASTType::ARRAY_LITERAL: {
      vector<ASTNodePtr> sub_tys{};
      sub_tys.reserve(p->_children.size());
      std::for_each(p->_children.begin(), p->_children.end(), [&sub_tys](const ASTNodePtr &e) {
        sub_tys.push_back(e->_ty);
      });
      p->_ty = create_ty(cs, Ty::ARRAY, sub_tys);
      break;
    }
      ////////////////////////// keywords ///////////////////////////
    case ASTType::IF: {
      auto cond = p->_children[0];
      if (0 != TypeSystem::CanImplicitCast(cs, create_ty(cs, Ty::BOOL), cond->_ty)) {
        report_error(cs, p, "Cannot convert type to bool");
      }
      break;
    }
      // TODO: cs->set_current_loop(pl) // case ASTType::LOOP:
      // TODO: cs->get_current_loop() // case ASTType::BREAK (or CONTINUE):
      ////////////////////////// others ///////////////////////////
    case ASTType::INTRINSIC: {
      analyze_intrinsic(cs, p);
      break;
    }
    case ASTType::IMPORT: {
      // TODO: determine whether to use class field or child ASTNode to store imported filename
      // auto rhs = p->_children[0];
      // str file = std::get<str>(rhs->_value);
      str file = p->_name;
      auto imported = Compiler::resolve_import(cs->_filename, file);
      if (imported.empty()) { report_error(cs, p, "Cannot import: " + file); }

      /// it might be already parsed
      vector<ASTFunctionPtr> imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
      if (imported_functions.empty()) {
        Compiler::ParseFile(imported[0]);
        imported_functions = CompilerSession::GetPublicFunctions(imported[0]);
      }
      for (auto &f: imported_functions) {
        cs->add_function(f);
        p->_children.push_back(f);
      }
      break;
    }
    case ASTType::PARENTHESIS:
      p->_ty = p->_children[0]->_ty;
      break;
    case ASTType::FUNC_CALL: {
      std::vector<ASTNodePtr> args(p->_children.begin(), p->_children.end());
      p->_children.clear();
      p->_children.push_back(ASTFunction::GetCallee(cs, p->_name, args));
      p->_ty = p->_children[0]->_ty;
      break;
    }
    case ASTType::TY: {
      ASTTyPtr pt = ast_cast<ASTTy>(p);
      TAN_ASSERT(pt);
      resolve_ty(cs, pt);
      break;
    }
      ////////////////////////// declarations ///////////////////////////
    case ASTType::ENUM_DECL: {
      // TODO: Analysis of enum types and values
      break;
    }
    case ASTType::FUNC_DECL: {
      /// add to function table
      if (p->_is_public || p->_is_external) { CompilerSession::AddPublicFunction(cs->_filename, p); }
      /// ...and to the internal function table
      cs->add_function(p);

      // TODO: function type

      /// add args to scope if function body exists
      size_t n = p->_children.size();
      size_t arg_end = n - 1 - !p->_is_external;
      for (size_t i = 1; i < arg_end; ++i) {
        if (!p->_is_external) { cs->add(p->_children[i]->_name, p->_children[i]); }
      }
      if (!p->_is_external) {
        /// new scope for function body
        auto f_body = p->_children[n - 1];
        if (!p->_is_external) {
          f_body->_scope = cs->push_scope();
        }
      }
      break;
    }
    case ASTType::ARG_DECL:
    case ASTType::VAR_DECL: {
      resolve_ty(cs, p->_ty);
      cs->add(p->_name, p);
      break;
    }
    case ASTType::STRUCT_DECL: {
      str struct_name = p->_children[0]->_name;
      auto ty = ast_create_ty(cs);
      ty->_tyty = Ty::STRUCT;

      auto forward_decl = cs->get(struct_name);
      // TODO: Check if struct name is in conflicts of variable/function names
      if (!forward_decl) {
        cs->add(struct_name, ty); /// add self to current scope
      } else {
        /// replace forward decl with self (even if this is a forward declaration too)
        cs->set(struct_name, ty);
      }

      /// resolve member names and types
      auto members = p->_children;
      size_t n = p->_children.size();
      ty->_member_names.reserve(n);
      ty->_children.reserve(n);
      for (size_t i = 0; i < n; ++i) {
        ASTNodePtr m = members[i];
        if (members[i]->_type == ASTType::VAR_DECL) { /// member variable without initial value
          ty->_children.push_back(m->_ty);
        } else if (members[i]->_type == ASTType::ASSIGN) { /// member variable with an initial value
          auto init_val = m->_children[1];
          m = m->_children[0];
          if (!is_ast_type_in(init_val->_type, TypeSystem::LiteralTypes)) {
            report_error(cs, p, "Invalid initial value of the member variable");
          }
          ty->_children.push_back(init_val->_ty); /// init_val->_ty->_default_value is set to the initial value
        } else { report_error(cs, p, "Invalid struct member"); }
        ty->_member_names.push_back(m->_name);
        ty->_member_indices[m->_name] = i;
      }
      resolve_ty(cs, ty);
      p->_ty = ty;
      break;
    }
      /////////////////////// trivially analysed /////////////////////////////
    default:
      break;
  }
}

} // namespace tanlang
