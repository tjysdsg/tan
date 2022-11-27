#include "codegen/code_generator.h"
#include "ast/ast_base.h"
#include "ast/type.h"
#include "ast/ast_context.h"
#include "ast/constructor.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "compiler/compiler_session.h"
#include "compiler/compiler.h"

namespace tanlang {

/**
 * \brief create_ty an `alloca` instruction in the beginning of a block.
 * \param block BasicBlock to insert to.
 * \param type Intended type to store.
 * \param name Name of the `alloca` instruction.
 * \param size size of the array if greater than 1
 */
static AllocaInst *create_block_alloca(BasicBlock *block, llvm::Type *type, size_t size = 1, const str &name = "");

class CodeGeneratorImpl {
public:
  CodeGeneratorImpl() = delete;

  explicit CodeGeneratorImpl(CompilerSession *cs, ASTContext *ctx)
      : _cs(cs), _ctx(ctx), _sm(cs->get_source_manager()) {}

  Value *codegen(ASTBase *p) {
    auto it = _llvm_value_cache.find(p);
    if (it != _llvm_value_cache.end()) {
      return it->second;
    }

    set_current_debug_location(p);

    Value *ret = nullptr;
    switch (p->get_node_type()) {
    case ASTNodeType::PROGRAM:
    case ASTNodeType::STATEMENT:
      ret = codegen_stmt(p);
      break;
    case ASTNodeType::ASSIGN:
      ret = codegen_assignment(p);
      break;
    case ASTNodeType::CAST:
      ret = codegen_cast(p);
      break;
    case ASTNodeType::BOP:
      ret = codegen_bop(p);
      break;
    case ASTNodeType::UOP:
      ret = codegen_uop(p);
      break;
    case ASTNodeType::RET:
      ret = codegen_return(p);
      break;
    case ASTNodeType::IMPORT:
      ret = codegen_import(p);
      break;
    case ASTNodeType::ARRAY_LITERAL:
    case ASTNodeType::INTEGER_LITERAL:
    case ASTNodeType::FLOAT_LITERAL:
    case ASTNodeType::CHAR_LITERAL:
    case ASTNodeType::BOOL_LITERAL:
    case ASTNodeType::STRING_LITERAL:
    case ASTNodeType::NULLPTR_LITERAL:
      ret = codegen_literals(p);
      break;
    case ASTNodeType::INTRINSIC:
      ret = codegen_intrinsic(ast_cast<Intrinsic>(p));
      break;
    case ASTNodeType::FUNC_DECL:
      ret = codegen_func_decl(ast_cast<FunctionDecl>(p));
      break;
    case ASTNodeType::FUNC_CALL:
      ret = codegen_func_call(p);
      break;
    case ASTNodeType::IF:
      ret = codegen_if(p);
      break;
    case ASTNodeType::CONTINUE:
    case ASTNodeType::BREAK:
      ret = codegen_break_continue(p);
      break;
    case ASTNodeType::LOOP:
      ret = codegen_loop(p);
      break;
    case ASTNodeType::VAR_DECL:
    case ASTNodeType::ARG_DECL:
      ret = codegen_var_arg_decl(p);
      break;
    case ASTNodeType::PARENTHESIS:
      ret = codegen_parenthesis(p);
      break;
    case ASTNodeType::VAR_REF:
      ret = codegen_var_ref(p);
      break;
    case ASTNodeType::ID:
      ret = codegen_identifier(p);
      break;
    case ASTNodeType::BOP_OR_UOP:
      ret = codegen_binary_or_unary(p);
      break;
    default:
      break;
    }

    return _llvm_value_cache[p] = ret;
  }

  /**
   * \brief Convert a value to from orig type to dest type.
   * \details Returns nullptr if failed to convert.
   * \param dest Destination type.
   * \param expr Original expression.
   * \return Converted value if convertible, otherwise `nullptr`. Note that the returned value is always rvalue. To
   * get an lvalue, create a temporary variable and store the value to it.
   * */
  llvm::Value *convert_llvm_type_to(Expr *expr, Type *dest) {
    auto *builder = _cs->_builder;

    /// load if lvalue
    Value *loaded = load_if_is_lvalue(expr);

    Type *orig = expr->get_type();

    bool is_pointer1 = orig->is_pointer();
    bool is_pointer2 = dest->is_pointer();

    /**
     * NOTE: check enum before checking int
     * */

    /// early return if types are the same
    if (orig == dest) {
      return loaded;
    };
    if (is_pointer1 && is_pointer2) {
      /// cast between pointer types (including pointers to pointers)
      return builder->CreateBitCast(loaded, to_llvm_type(dest));
    } else if ((orig->is_enum() && dest->is_int()) || (dest->is_enum() && orig->is_int())) {
      return builder->CreateZExtOrTrunc(loaded, to_llvm_type(dest));
    } else if ((orig->is_int() || orig->is_char()) && (dest->is_char() || dest->is_int())) { /// between int
      return builder->CreateZExtOrTrunc(loaded, to_llvm_type(dest));
    } else if (orig->is_int() && dest->is_float()) { /// int to float/double
      if (orig->is_unsigned()) {
        return builder->CreateUIToFP(loaded, to_llvm_type(dest));
      } else {
        return builder->CreateSIToFP(loaded, to_llvm_type(dest));
      }
    } else if (orig->is_float() && dest->is_int()) { /// float/double to int
      if (orig->is_unsigned()) {
        return builder->CreateFPToUI(loaded, to_llvm_type(dest));
      } else {
        return builder->CreateFPToSI(loaded, to_llvm_type(dest));
      }
    } else if (orig->is_float() && dest->is_float()) { /// float <-> double
      return builder->CreateFPCast(loaded, to_llvm_type(dest));
    } else if (orig->is_bool() && dest->is_int()) { /// bool to int
      return builder->CreateZExtOrTrunc(loaded, to_llvm_type(dest));
    } else if (orig->is_bool() && dest->is_float()) { /// bool to float
      return builder->CreateUIToFP(loaded, to_llvm_type(dest));
    } else if (dest->is_bool()) {
      if (orig->is_float()) { /// float to bool
        if (orig->get_size_bits() == 32) {
          return builder->CreateFCmpONE(loaded, ConstantFP::get(builder->getFloatTy(), 0.0f));
        } else {
          return builder->CreateFCmpONE(loaded, ConstantFP::get(builder->getDoubleTy(), 0.0f));
        }
      } else if (orig->is_pointer()) { /// pointer to bool
        size_t s1 = _cs->get_ptr_size();
        loaded = builder->CreatePtrToInt(loaded, builder->getIntNTy((unsigned)s1));
        return builder->CreateICmpNE(loaded, ConstantInt::get(builder->getIntNTy((unsigned)s1), 0, false));
      } else if (orig->is_int()) { /// int to bool
        auto *t = (PrimitiveType *)orig;
        return builder->CreateICmpNE(loaded,
                                     ConstantInt::get(builder->getIntNTy((unsigned)t->get_size_bits()), 0, false));
      }
    } else if (orig->is_string() && dest->is_pointer()) { /// string to pointer, don't need to do anything
      return loaded;
    } else if (orig->is_array() && dest->is_pointer()) { /// array to pointer, don't need to do anything
      return loaded;
    } else if (orig->is_array() && dest->is_string()) { /// array to string, don't need to do anything
      return loaded;
    }

    error(expr, "Cannot perform type conversion");
  }

  /**
   * \brief Create a load instruction if the type is lvalue. Otherwise return the original value.
   */
  llvm::Value *load_if_is_lvalue(Expr *expr) {
    Value *val = _llvm_value_cache[expr];
    TAN_ASSERT(val);

    if (expr->is_lvalue()) {
      return _cs->_builder->CreateLoad(val);
    }
    return val;
  }

  llvm::Type *to_llvm_type(Type *p) {
    TAN_ASSERT(p);
    TAN_ASSERT(!p->is_ref());

    auto it = _llvm_type_cache.find(p);
    if (it != _llvm_type_cache.end()) {
      return it->second;
    }

    auto *builder = _cs->_builder;
    llvm::Type *ret = nullptr;

    if (p->is_primitive()) { /// primitive types
      int size_bits = ((PrimitiveType *)p)->get_size_bits();
      if (p->is_int()) {
        ret = builder->getIntNTy((unsigned)size_bits);
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
      // TODO IMPORTANT: ret = TypeSystem::to_llvm_type(cs, p->get_sub_types()[0]);
      TAN_ASSERT(false);
    } else if (p->is_struct()) { /// struct
      auto member_types = ((StructType *)p)->get_member_types();
      vector<llvm::Type *> elements(member_types.size(), nullptr);
      for (size_t i = 0; i < member_types.size(); ++i) {
        elements[i] = to_llvm_type(member_types[i]);
      }
      ret = llvm::StructType::create(elements, p->get_typename());
    } else if (p->is_array()) { /// array as pointer
      auto *e_type = to_llvm_type(((ArrayType *)p)->get_element_type());
      ret = e_type->getPointerTo();
    } else if (p->is_pointer()) { /// pointer
      auto *e_type = to_llvm_type(((PointerType *)p)->get_pointee());
      ret = e_type->getPointerTo();
    } else {
      TAN_ASSERT(false);
    }

    _llvm_type_cache[p] = ret;
    return ret;
  }

  llvm::Metadata *to_llvm_metadata(Type *p) {
    TAN_ASSERT(p);
    TAN_ASSERT(!p->is_ref());

    auto it = _llvm_metadata_cache.find(p);
    if (it != _llvm_metadata_cache.end()) {
      return it->second;
    }

    DIType *ret = nullptr;
    auto *tm = Compiler::GetDefaultTargetMachine();

    if (p->is_primitive()) { /// primitive types
      unsigned dwarf_encoding = 0;
      auto *pp = (PrimitiveType *)p;
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

      ret = _cs->_di_builder->createBasicType(p->get_typename(), (uint64_t)size_bits, dwarf_encoding);
    } else if (p->is_string()) { /// str as char*
      auto *e_di_type = _cs->_di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
      ret = _cs->_di_builder->createPointerType(e_di_type, tm->getPointerSizeInBits(0),
                                                (unsigned)tm->getPointerSizeInBits(0), llvm::None, p->get_typename());
    } else if (p->is_enum()) { /// enums
      // TODO IMPORTANT
    } else if (p->is_struct()) { /// struct
      DIFile *di_file = _cs->get_di_file();
      auto member_types = ((StructType *)p)->get_member_types();
      vector<Metadata *> elements(member_types.size(), nullptr);
      for (size_t i = 1; i < member_types.size(); ++i) {
        elements[i] = to_llvm_metadata(member_types[i]);
      }
      ret = _cs->_di_builder->createStructType(
          _cs->get_current_di_scope(), p->get_typename(), di_file,
          0, // TODO IMPORTANT: (unsigned) cs->get_source_manager()->get_line(p->loc()),
          0, // TODO IMPORTANT: p->get_size_bits(),
          0, // TODO IMPORTANT: p->get_align_bits(),
          DINode::DIFlags::FlagZero, nullptr, _cs->_di_builder->getOrCreateArray(elements), 0, nullptr,
          p->get_typename());
    } else if (p->is_array()) { /// array as pointer
      auto *sub = to_llvm_metadata(((ArrayType *)p)->get_element_type());
      ret = _cs->_di_builder->createPointerType((DIType *)sub, tm->getPointerSizeInBits(0),
                                                (unsigned)tm->getPointerSizeInBits(0), llvm::None, p->get_typename());
    } else if (p->is_pointer()) { /// pointer
      auto *sub = to_llvm_metadata(((PointerType *)p)->get_pointee());
      ret = _cs->_di_builder->createPointerType((DIType *)sub, tm->getPointerSizeInBits(0),
                                                (unsigned)tm->getPointerSizeInBits(0), llvm::None, p->get_typename());
    } else {
      TAN_ASSERT(false);
    }

    _llvm_metadata_cache[p] = ret;
    return ret;
  }

  llvm::DISubroutineType *create_function_debug_info_type(llvm::Metadata *ret, vector<llvm::Metadata *> args) {
    vector<Metadata *> types{ret};
    types.reserve(args.size());
    types.insert(types.begin() + 1, args.begin(), args.end());
    //  return cs->_di_builder
    //    ->createSubroutineType(cs->_di_builder->getOrCreateTypeArray(types), DINode::FlagZero,
    //    llvm::dwarf::DW_CC_normal);
    return _cs->_di_builder->createSubroutineType(_cs->_di_builder->getOrCreateTypeArray(types));
  }

private:
  CompilerSession *_cs = nullptr;
  ASTContext *_ctx = nullptr;
  SourceManager *_sm = nullptr;

  /// avoid creating duplicated llvm::Type and llvm::Metadata
  umap<Type *, llvm::Type *> _llvm_type_cache{};
  umap<Type *, llvm::Metadata *> _llvm_metadata_cache{};
  umap<ASTBase *, llvm::Value *> _llvm_value_cache{};

private:
  [[noreturn]] void error(ASTBase *p, const str &message) {
    Error err(_cs->_filename, _sm->get_token(p->loc()), message);
    err.raise();
  }

  DebugLoc debug_loc_of_node(ASTBase *p, MDNode *scope = nullptr) {
    return DebugLoc::get(_sm->get_line(p->loc()), _sm->get_col(p->loc()), scope);
  }

  Value *codegen_func_call(ASTBase *_p) {
    auto p = ast_cast<FunctionCall>(_p);

    FunctionDecl *callee = p->_callee;
    size_t n = callee->get_n_args();

    /// args
    vector<Value *> arg_vals;
    for (size_t i = 0; i < n; ++i) {
      auto actual_arg = p->_args[i];
      auto *a = codegen(actual_arg);
      if (!a) {
        error(actual_arg, "Invalid function call argument");
      }

      /// implicit cast
      auto expected_ty = callee->get_arg_type(i);
      a = convert_llvm_type_to(actual_arg, expected_ty);
      arg_vals.push_back(a);
    }
    return _cs->_builder->CreateCall(_llvm_value_cache[callee], arg_vals);
  }

  Value *codegen_func_prototype(FunctionDecl *p, bool import = false) {
    /// set function arg types
    vector<llvm::Type *> arg_types{};
    for (size_t i = 0; i < p->get_n_args(); ++i) {
      arg_types.push_back(to_llvm_type(p->get_arg_type(i)));
    }

    /// return type
    llvm::Type *ret_type = to_llvm_type(p->get_ret_ty());

    /// create function prototype
    llvm::FunctionType *FT = llvm::FunctionType::get(ret_type, arg_types, false);
    auto linkage = Function::InternalLinkage;
    if (p->is_external()) {
      linkage = Function::ExternalWeakLinkage;
    }
    if (p->is_public()) {
      if (import) {
        linkage = Function::ExternalWeakLinkage;
      } else {
        linkage = Function::ExternalLinkage;
      }
    }
    Function *func = Function::Create(FT, linkage, p->get_name(), _cs->get_module());
    func->setCallingConv(llvm::CallingConv::C);
    return func;
  }

  Value *codegen_func_decl(FunctionDecl *p) {
    auto *builder = _cs->_builder;

    auto ret_ty = p->get_ret_ty();
    Metadata *ret_meta = to_llvm_metadata(ret_ty);

    /// get function name
    str func_name = p->get_name();
    /// rename to "tan_main", as it will be called by the real main function in runtime/main.cpp
    if (func_name == "main") {
      p->set_name(func_name = "tan_main");
      p->set_public(true);
    }

    /// generate prototype
    auto *F = (Function *)codegen_func_prototype(p);

    /// set function arg types
    vector<Metadata *> arg_metas;
    for (size_t i = 0; i < p->get_n_args(); ++i) {
      auto ty = p->get_arg_type(i);
      arg_metas.push_back(to_llvm_metadata(ty));
    }

    /// function implementation
    if (!p->is_external()) {
      /// create a new basic block to start insertion into
      BasicBlock *main_block = BasicBlock::Create(*_cs->get_context(), "func_entry", F);
      builder->SetInsertPoint(main_block);

      /// debug information
      DIScope *di_scope = _cs->get_current_di_scope();
      auto *di_file = _cs->get_di_file();
      auto *di_func_t = create_function_debug_info_type(ret_meta, arg_metas);
      DISubprogram *subprogram = _cs->_di_builder->createFunction(
          di_scope, func_name, func_name, di_file, _sm->get_line(p->loc()), di_func_t, _sm->get_col(p->loc()),
          DINode::FlagPrototyped, DISubprogram::SPFlagDefinition, nullptr, nullptr, nullptr);
      F->setSubprogram(subprogram);
      _cs->push_di_scope(subprogram);

      /// add all function arguments to scope
      size_t i = 0;
      for (auto &a : F->args()) {
        auto arg_name = p->get_arg_name(i);
        auto *arg_val = codegen(p->get_arg_decls()[i]);
        builder->CreateStore(&a, arg_val);

        /// create a debug descriptor for the arguments
        auto *arg_meta = to_llvm_metadata(p->get_arg_type(i));
        llvm::DILocalVariable *di_arg = _cs->_di_builder->createParameterVariable(
            subprogram, arg_name, (unsigned)i + 1, di_file, _sm->get_line(p->loc()), (DIType *)arg_meta, true);
        _cs->_di_builder->insertDeclare(arg_val, di_arg, _cs->_di_builder->createExpression(),
                                        debug_loc_of_node(p->get_arg_decls()[i], subprogram),
                                        builder->GetInsertBlock());
        ++i;
      }

      /// generate function body
      codegen(p->get_body());

      /// create a return instruction if there is none, the return value is the default value of the return type
      auto *trailing_block = builder->GetInsertBlock();
      if (trailing_block->sizeWithoutDebug() == 0 || !trailing_block->back().isTerminator()) {
        if (ret_ty->is_void()) {
          builder->CreateRetVoid();
        } else {
          auto *ret_val = codegen_type_instantiation(ret_ty);
          TAN_ASSERT(ret_val);
          builder->CreateRet(ret_val);
        }
      }
      _cs->pop_di_scope();
    }

    return F;
  }

  void set_current_debug_location(ASTBase *p) {
    _cs->set_current_debug_location(_sm->get_line(p->loc()), _sm->get_col(p->loc()));
  }

  Value *codegen_bnot(ASTBase *_p) {
    auto p = ast_cast<UnaryOperator>(_p);

    auto *builder = _cs->_builder;

    auto *rhs = codegen(p->get_rhs());
    if (!rhs) {
      error(p, "Invalid operand");
    }
    if (p->get_rhs()->is_lvalue()) {
      rhs = builder->CreateLoad(rhs);
    }
    return builder->CreateNot(rhs);
  }

  Value *codegen_lnot(ASTBase *_p) {
    auto p = ast_cast<UnaryOperator>(_p);

    auto *builder = _cs->_builder;

    auto *rhs = codegen(p->get_rhs());

    if (!rhs) {
      error(p, "Invalid operand");
    }

    if (p->get_rhs()->is_lvalue()) {
      rhs = builder->CreateLoad(rhs);
    }
    /// get value size in bits
    auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
    if (rhs->getType()->isFloatingPointTy()) {
      return builder->CreateFCmpOEQ(rhs, ConstantFP::get(builder->getFloatTy(), 0.0f));
    } else if (rhs->getType()->isSingleValueType()) {
      return builder->CreateICmpEQ(rhs, ConstantInt::get(builder->getIntNTy((unsigned)size_in_bits), 0, false));
    }

    error(p, "Invalid operand");
  }

  Value *codegen_return(ASTBase *_p) {
    auto p = ast_cast<Return>(_p);

    auto *builder = _cs->_builder;

    auto rhs = p->get_rhs();
    if (rhs) { /// return with value
      Value *result = codegen(rhs);
      if (rhs->is_lvalue()) {
        result = builder->CreateLoad(result, "ret");
      }
      builder->CreateRet(result);
    } else { /// return void
      builder->CreateRetVoid();
    }
    return nullptr;
  }

  Value *codegen_var_arg_decl(ASTBase *_p) {
    auto p = ast_cast<Decl>(_p);

    auto *builder = _cs->_builder;

    llvm::Type *type = to_llvm_type(p->get_type());
    auto *ret = create_block_alloca(builder->GetInsertBlock(), type, 1, p->get_name());

    /// default value of only var declaration
    if (p->get_node_type() == ASTNodeType::VAR_DECL) {
      auto *default_value = codegen_type_instantiation(p->get_type());
      if (!default_value) {
        error(p, "Fail to instantiate this type");
      }
      builder->CreateStore(default_value, ret);
    }

    /// debug info
    {
      auto *di_builder = _cs->_di_builder;
      auto *curr_di_scope = _cs->get_current_di_scope();
      auto *arg_meta = to_llvm_metadata(p->get_type());
      auto *di_arg = di_builder->createAutoVariable(curr_di_scope, p->get_name(), _cs->get_di_file(),
                                                    _sm->get_line(p->loc()), (DIType *)arg_meta);
      di_builder->insertDeclare(ret, di_arg, _cs->_di_builder->createExpression(),
                                llvm::DebugLoc::get(_sm->get_line(p->loc()), _sm->get_col(p->loc()), curr_di_scope),
                                builder->GetInsertBlock());
    }
    return ret;
  }

  Value *codegen_address_of(ASTBase *_p) {
    auto p = ast_cast<UnaryOperator>(_p);

    if (!p->get_rhs()->is_lvalue()) {
      error(p, "Cannot get address of rvalue");
    }

    return codegen(p->get_rhs());
  }

  Value *codegen_parenthesis(ASTBase *_p) {
    auto p = ast_cast<Parenthesis>(_p);
    return codegen(p->get_sub());
  }

  Value *codegen_import(ASTBase *_p) {
    auto p = ast_cast<Import>(_p);

    for (FunctionDecl *f : p->get_imported_funcs()) {
      /// do nothing for already defined intrinsics
      auto *func = _cs->get_module()->getFunction(f->get_name());
      if (func) {
        _llvm_value_cache[f] = func;
      }
      _llvm_value_cache[f] = codegen_func_prototype(f);
    }

    return nullptr;
  }

  Value *codegen_intrinsic(Intrinsic *p) {
    Value *ret = nullptr;
    switch (p->get_intrinsic_type()) {
    /// trivial codegen
    case IntrinsicType::GET_DECL:
    case IntrinsicType::LINENO:
    case IntrinsicType::NOOP:
    case IntrinsicType::ABORT:
    case IntrinsicType::STACK_TRACE:
    case IntrinsicType::FILENAME: {
      ret = codegen(p->get_sub());
      break;
    }
    default:
      break;
    }
    return ret;
  }

  Value *codegen_constructor(Constructor *p) {
    Value *ret = nullptr;
    switch (p->get_type()) {
    case ConstructorType::BASIC:
      ret = codegen(((BasicConstructor *)p)->get_value());
      break;
    case ConstructorType::STRUCT: {
      auto *ctr = (StructConstructor *)p;
      vector<Constructor *> sub_ctrs = ctr->get_member_constructors();
      vector<Constant *> values{};
      values.reserve(sub_ctrs.size());
      std::transform(sub_ctrs.begin(), sub_ctrs.end(), values.begin(),
                     [&](Constructor *c) { return (Constant *)codegen_constructor(c); });
      ret = ConstantStruct::get((llvm::StructType *)to_llvm_type(ctr->get_struct_type()), values);
      break;
    }
    default:
      TAN_ASSERT(false);
    }
    return ret;
  }

  Value *codegen_type_instantiation(Type *p) {
    // TODO: TAN_ASSERT(p->get_constructor());
    TAN_ASSERT(!p->is_ref());

    Value *ret = nullptr;
    if (p->is_primitive()) { /// primitive types
      int size_bits = ((PrimitiveType *)p)->get_size_bits();

      if (p->is_int()) {
        ret = codegen_constructor(
            BasicConstructor::CreateIntegerConstructor(SrcLoc(0), 0, (size_t)size_bits, p->is_unsigned()));
      } else if (p->is_char()) {
        ret = codegen_constructor(BasicConstructor::CreateCharConstructor(SrcLoc(0), 0));
      } else if (p->is_bool()) {
        ret = codegen_constructor(BasicConstructor::CreateBoolConstructor(SrcLoc(0), false));
      } else if (p->is_float()) {
        ret = codegen_constructor(BasicConstructor::CreateFPConstructor(SrcLoc(0), 0, (size_t)size_bits));
      } else {
        TAN_ASSERT(false);
      }
    } else if (p->is_string()) { /// str as char*
      ret = codegen_constructor(BasicConstructor::CreateStringConstructor(SrcLoc(0), ""));
    } else if (p->is_enum()) { /// enums
      // TODO IMPORTANT
      TAN_ASSERT(false);
    } else if (p->is_struct()) { /// struct
      // TODO: use codegen_constructor()
      auto member_types = ((StructType *)p)->get_member_types();
      vector<Constant *> values(member_types.size(), nullptr);
      for (size_t i = 0; i < member_types.size(); ++i) {
        values[i] = (llvm::Constant *)codegen_type_instantiation(member_types[i]);
      }
      ret = ConstantStruct::get((llvm::StructType *)to_llvm_type(p), values);
    } else if (p->is_array()) { /// array as pointer
      ret = codegen_constructor(
          BasicConstructor::CreateArrayConstructor(SrcLoc(0), ((ArrayType *)p)->get_element_type()));
    } else if (p->is_pointer()) { /// the pointer literal is nullptr
      ret = codegen_constructor(
          BasicConstructor::CreateNullPointerConstructor(SrcLoc(0), ((PointerType *)p)->get_pointee()));
    } else {
      TAN_ASSERT(false);
    }

    return ret;
  }

  Value *codegen_literals(ASTBase *_p) {
    auto p = ast_cast<Literal>(_p);

    auto *builder = _cs->_builder;

    llvm::Type *type = to_llvm_type(p->get_type());
    Value *ret = nullptr;
    Type *ptype = p->get_type();
    if (ptype->is_primitive()) { /// primitive types
      int size_bits = ((PrimitiveType *)ptype)->get_size_bits();

      if (ptype->is_char()) { // NOTE: must be before is_int() check because char is technically an integer
        ret = ConstantInt::get(type, ast_cast<CharLiteral>(p)->get_value());
      } else if (ptype->is_int()) {
        auto pp = ast_cast<IntegerLiteral>(p);
        ret = ConstantInt::get(_cs->_builder->getIntNTy((unsigned)size_bits), pp->get_value(), !pp->is_unsigned());
      } else if (ptype->is_bool()) {
        auto pp = ast_cast<BoolLiteral>(p);
        ret = ConstantInt::get(type, (uint64_t)pp->get_value());
      } else if (ptype->is_float()) {
        ret = ConstantFP::get(type, ast_cast<FloatLiteral>(p)->get_value());
      } else {
        TAN_ASSERT(false);
      }
    } else if (ptype->is_string()) { /// str as char*
      ret = builder->CreateGlobalStringPtr(ast_cast<StringLiteral>(p)->get_value());
    } else if (ptype->is_enum()) { /// enums
      // TODO IMPORTANT
      TAN_ASSERT(false);
    } else if (ptype->is_struct()) { /// struct
      // TODO: Implement struct literal
      TAN_ASSERT(false);
    } else if (ptype->is_array()) { /// array as pointer
      auto arr = ast_cast<ArrayLiteral>(p);

      /// element type
      auto elements = arr->get_elements();
      auto *e_type = to_llvm_type(((ArrayType *)ptype)->get_element_type());

      /// codegen element values
      size_t n = elements.size();
      ret = create_block_alloca(builder->GetInsertBlock(), e_type, n, "const_array");
      for (size_t i = 0; i < n; ++i) {
        auto *idx = builder->getInt32((unsigned)i);
        auto *e_val = codegen(elements[i]);
        auto *e_ptr = builder->CreateGEP(ret, idx);
        builder->CreateStore(e_val, e_ptr);
      }
    } else if (ptype->is_pointer()) { /// the pointer literal is nullptr
      ret = ConstantPointerNull::get((llvm::PointerType *)type);
    } else {
      TAN_ASSERT(false);
    }

    return ret;
  }

  Value *codegen_stmt(ASTBase *_p) {
    auto p = ast_cast<CompoundStmt>(_p);

    for (const auto &e : p->get_children()) {
      codegen(e);
    }
    return nullptr;
  }

  Value *codegen_uop(ASTBase *_p) {
    auto p = ast_cast<UnaryOperator>(_p);
    Value *ret = nullptr;

    auto *builder = _cs->_builder;

    auto rhs = p->get_rhs();
    switch (p->get_op()) {
    case UnaryOpKind::LNOT:
      ret = codegen_lnot(p);
      break;
    case UnaryOpKind::BNOT:
      ret = codegen_bnot(p);
      break;
    case UnaryOpKind::ADDRESS_OF:
      ret = codegen_address_of(p);
      break;
    case UnaryOpKind::PTR_DEREF:
      ret = codegen_ptr_deref(p);
      break;
    case UnaryOpKind::PLUS:
      ret = codegen(rhs);
      break;
    case UnaryOpKind::MINUS: {
      auto *r = codegen(rhs);
      if (rhs->is_lvalue()) {
        r = builder->CreateLoad(r);
      }
      if (r->getType()->isFloatingPointTy()) {
        ret = builder->CreateFNeg(r);
      } else {
        ret = builder->CreateNeg(r);
      }
      break;
    }
    default:
      TAN_ASSERT(false);
      break;
    }
    return ret;
  }

  Value *codegen_bop(ASTBase *_p) {
    auto p = ast_cast<BinaryOperator>(_p);
    Value *ret = nullptr;

    switch (p->get_op()) {
    case BinaryOpKind::SUM:
    case BinaryOpKind::SUBTRACT:
    case BinaryOpKind::MULTIPLY:
    case BinaryOpKind::DIVIDE:
    case BinaryOpKind::MOD:
      ret = codegen_arithmetic(p);
      break;
    case BinaryOpKind::BAND:
    case BinaryOpKind::LAND:
    case BinaryOpKind::BOR:
    case BinaryOpKind::LOR:
    case BinaryOpKind::XOR:
      ret = codegen_relop(p);
      break;
    case BinaryOpKind::GT:
    case BinaryOpKind::GE:
    case BinaryOpKind::LT:
    case BinaryOpKind::LE:
    case BinaryOpKind::EQ:
    case BinaryOpKind::NE:
      ret = codegen_comparison(p);
      break;
    case BinaryOpKind::MEMBER_ACCESS:
      ret = codegen_member_access(ast_cast<MemberAccess>(p));
      break;
    default:
      TAN_ASSERT(false);
      break;
    }

    return ret;
  }

  Value *codegen_assignment(ASTBase *_p) {
    auto p = ast_cast<Assignment>(_p);

    auto *builder = _cs->_builder;

    /// codegen the lhs and rhs
    auto *lhs = ast_cast<Expr>(p->get_lhs());
    auto *rhs = p->get_rhs();

    // type of lhs is the same as type of the assignment
    if (!lhs->is_lvalue()) {
      error(lhs, "Value can only be assigned to an lvalue");
    }

    Value *from = codegen(rhs);
    from = load_if_is_lvalue(rhs);
    Value *to = codegen(lhs);
    TAN_ASSERT(from && to);

    builder->CreateStore(from, to);
    return to;
  }

  Value *codegen_arithmetic(ASTBase *_p) {
    auto p = ast_cast<BinaryOperator>(_p);

    auto *builder = _cs->_builder;

    /// binary operator
    auto *lhs = p->get_lhs();
    auto *rhs = p->get_rhs();
    Value *l = codegen(lhs);
    Value *r = codegen(rhs);
    r = load_if_is_lvalue(rhs);
    l = load_if_is_lvalue(lhs);

    Value *ret = nullptr;
    if (l->getType()->isFloatingPointTy()) {
      /// float arithmetic
      switch (p->get_op()) {
      case BinaryOpKind::MULTIPLY:
        ret = builder->CreateFMul(l, r, "mul_tmp");
        break;
      case BinaryOpKind::DIVIDE:
        ret = builder->CreateFDiv(l, r, "div_tmp");
        break;
      case BinaryOpKind::SUM:
        ret = builder->CreateFAdd(l, r, "sum_tmp");
        break;
      case BinaryOpKind::SUBTRACT:
        ret = builder->CreateFSub(l, r, "sub_tmp");
        break;
      case BinaryOpKind::MOD:
        ret = builder->CreateFRem(l, r, "mod_tmp");
        break;
      default:
        TAN_ASSERT(false);
        break;
      }
    } else {
      /// integer arithmetic
      switch (p->get_op()) {
      case BinaryOpKind::MULTIPLY:
        ret = builder->CreateMul(l, r, "mul_tmp");
        break;
      case BinaryOpKind::DIVIDE: {
        auto ty = lhs->get_type();
        if (ty->is_unsigned()) {
          ret = builder->CreateUDiv(l, r, "div_tmp");
        } else {
          ret = builder->CreateSDiv(l, r, "div_tmp");
        }
        break;
      }
      case BinaryOpKind::SUM:
        ret = builder->CreateAdd(l, r, "sum_tmp");
        break;
      case BinaryOpKind::SUBTRACT:
        ret = builder->CreateSub(l, r, "sub_tmp");
        break;
      case BinaryOpKind::MOD: {
        auto ty = lhs->get_type();
        if (ty->is_unsigned()) {
          ret = builder->CreateURem(l, r, "mod_tmp");
        } else {
          ret = builder->CreateSRem(l, r, "mod_tmp");
        }
        break;
      }
      default:
        TAN_ASSERT(false);
        break;
      }
    }

    TAN_ASSERT(ret);
    return ret;
  }

  Value *codegen_comparison(ASTBase *_p) {
    auto p = ast_cast<BinaryOperator>(_p);

    auto *builder = _cs->_builder;

    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();
    Value *l = codegen(lhs);
    Value *r = codegen(rhs);

    bool is_signed = !lhs->get_type()->is_unsigned();
    r = load_if_is_lvalue(rhs);
    l = load_if_is_lvalue(lhs);

    Value *ret = nullptr;
    if (l->getType()->isFloatingPointTy()) {
      switch (p->get_op()) {
      case BinaryOpKind::EQ:
        ret = builder->CreateFCmpOEQ(l, r, "eq");
        break;
      case BinaryOpKind::NE:
        ret = builder->CreateFCmpONE(l, r, "ne");
        break;
      case BinaryOpKind::GT:
        ret = builder->CreateFCmpOGT(l, r, "gt");
        break;
      case BinaryOpKind::GE:
        ret = builder->CreateFCmpOGE(l, r, "ge");
        break;
      case BinaryOpKind::LT:
        ret = builder->CreateFCmpOLT(l, r, "lt");
        break;
      case BinaryOpKind::LE:
        ret = builder->CreateFCmpOLE(l, r, "le");
        break;
      default:
        TAN_ASSERT(false);
        break;
      }
    } else {
      switch (p->get_op()) {
      case BinaryOpKind::EQ:
        ret = builder->CreateICmpEQ(l, r, "eq");
        break;
      case BinaryOpKind::NE:
        ret = builder->CreateICmpNE(l, r, "ne");
        break;
      case BinaryOpKind::GT:
        if (is_signed) {
          ret = builder->CreateICmpSGT(l, r, "gt");
        } else {
          ret = builder->CreateICmpUGT(l, r, "gt");
        }
        break;
      case BinaryOpKind::GE:
        if (is_signed) {
          ret = builder->CreateICmpSGE(l, r, "ge");
        } else {
          ret = builder->CreateICmpUGE(l, r, "ge");
        }
        break;
      case BinaryOpKind::LT:
        if (is_signed) {
          ret = builder->CreateICmpSLT(l, r, "lt");
        } else {
          ret = builder->CreateICmpULT(l, r, "lt");
        }
        break;
      case BinaryOpKind::LE:
        if (is_signed) {
          ret = builder->CreateICmpSLE(l, r, "le");
        } else {
          ret = builder->CreateICmpULE(l, r, "le");
        }
        break;
      default:
        TAN_ASSERT(false);
        break;
      }
    }

    TAN_ASSERT(ret);
    return ret;
  }

  Value *codegen_relop(ASTBase *_p) {
    auto p = ast_cast<BinaryOperator>(_p);

    auto *builder = _cs->_builder;

    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();
    Value *l = codegen(lhs);
    Value *r = codegen(rhs);

    r = load_if_is_lvalue(rhs);
    l = load_if_is_lvalue(lhs);

    Value *ret = nullptr;
    switch (p->get_op()) {
    case BinaryOpKind::BAND:
      ret = builder->CreateAnd(l, r, "binary_and");
      break;
    case BinaryOpKind::LAND:
      ret = builder->CreateAnd(l, r, "logical_and");
      break;
    case BinaryOpKind::BOR:
      ret = builder->CreateOr(l, r, "binary_or");
      break;
    case BinaryOpKind::LOR:
      ret = builder->CreateOr(l, r, "logical_or");
      break;
    case BinaryOpKind::XOR:
      ret = builder->CreateXor(l, r, "logical_or");
      break;
    default:
      TAN_ASSERT(false);
      break;
    }

    TAN_ASSERT(ret);
    return ret;
  }

  Value *codegen_cast(ASTBase *_p) {
    auto p = ast_cast<Cast>(_p);

    auto *builder = _cs->_builder;

    auto lhs = p->get_lhs();
    auto *dest_type = to_llvm_type(p->get_type());

    Value *val = codegen(lhs);
    TAN_ASSERT(val);

    Value *ret = nullptr;

    val = convert_llvm_type_to(lhs, p->get_type()); // lvalue will be loaded here
    if (lhs->is_lvalue()) {
      ret = create_block_alloca(builder->GetInsertBlock(), dest_type, 1, "casted");
      builder->CreateStore(val, ret);
    } else {
      ret = val;
    }

    return ret;
  }

  Value *codegen_var_ref(ASTBase *_p) {
    auto p = ast_cast<VarRef>(_p);
    return codegen(p->get_referred());
  }

  Value *codegen_identifier(ASTBase *_p) {
    auto p = ast_cast<Identifier>(_p);

    switch (p->get_id_type()) {
    case IdentifierType::ID_VAR_DECL:
      return codegen(p->get_var_ref());
    case IdentifierType::ID_TYPE_DECL:
    default:
      TAN_ASSERT(false);
      break;
    }
  }

  Value *codegen_binary_or_unary(ASTBase *_p) {
    auto p = ast_cast<BinaryOrUnary>(_p);
    return codegen(p->get_expr_ptr());
  }

  Value *codegen_break_continue(ASTBase *_p) {
    auto *p = ast_cast<BreakContinue>(_p);
    auto *builder = _cs->_builder;
    auto loop = p->get_parent_loop();
    TAN_ASSERT(loop);

    auto s = loop->_loop_start;
    auto e = loop->_loop_end;
    TAN_ASSERT(s);
    TAN_ASSERT(e);

    if (p->get_node_type() == ASTNodeType::BREAK) {
      builder->CreateBr(e);
    } else if (p->get_node_type() == ASTNodeType::CONTINUE) {
      builder->CreateBr(s);
    } else {
      TAN_ASSERT(false);
    }
    return nullptr;
  }

  Value *codegen_loop(ASTBase *_p) {
    auto p = ast_cast<Loop>(_p);
    auto *builder = _cs->_builder;

    if (p->_loop_type == ASTLoopType::WHILE) {
      /*
       * Results should like this:
       *
       * ...
       * loop:
       *    exit condition check, goto 'loop_body' or 'after_loop'
       * loop_body:
       *    ...
       *    goto 'loop'
       * after_loop:
       *    ...
       * */

      Function *func = builder->GetInsertBlock()->getParent();

      /// make sure to set _loop_start and _loop_end before generating loop_body, cuz break and continue statements
      /// use these two (get_loop_start() and get_loop_end())
      p->_loop_start = BasicBlock::Create(*_cs->get_context(), "loop", func);
      BasicBlock *loop_body = BasicBlock::Create(*_cs->get_context(), "loop_body", func);
      p->_loop_end = BasicBlock::Create(*_cs->get_context(), "after_loop", func);

      /// start loop
      // create a br instruction if there is no terminator instruction at the end of this block
      if (!builder->GetInsertBlock()->back().isTerminator()) {
        builder->CreateBr(p->_loop_start);
      }

      /// condition
      builder->SetInsertPoint(p->_loop_start);
      auto *cond = codegen(p->get_predicate());
      if (!cond) {
        error(p, "Expected a condition expression");
      }
      builder->CreateCondBr(cond, loop_body, p->_loop_end);

      /// loop body
      builder->SetInsertPoint(loop_body);
      codegen(p->get_body());

      /// go back to the start of the loop
      // create a br instruction if there is no terminator instruction at the end of this block
      if (!builder->GetInsertBlock()->back().isTerminator()) {
        builder->CreateBr(p->_loop_start);
      }

      /// end loop
      builder->SetInsertPoint(p->_loop_end);
    } else {
      TAN_ASSERT(false);
    }

    return nullptr;
  }

  Value *codegen_if(ASTBase *_p) {
    auto p = ast_cast<If>(_p);

    auto *builder = _cs->_builder;

    Function *func = builder->GetInsertBlock()->getParent();
    BasicBlock *merge_bb = BasicBlock::Create(*_cs->get_context(), "endif");
    size_t n = p->get_num_branches();

    /// create basic blocks
    vector<BasicBlock *> cond_blocks(n);
    vector<BasicBlock *> then_blocks(n);
    for (size_t i = 0; i < n; ++i) {
      cond_blocks[i] = BasicBlock::Create(*_cs->get_context(), "cond", func);
      then_blocks[i] = BasicBlock::Create(*_cs->get_context(), "branch", func);
    }

    /// codegen branches
    builder->CreateBr(cond_blocks[0]);
    for (size_t i = 0; i < n; ++i) {
      /// condition
      builder->SetInsertPoint(cond_blocks[i]);

      Expr *cond = p->get_predicate(i);
      if (!cond) {              /// else clause, immediately go to then block
        TAN_ASSERT(i == n - 1); /// only the last branch can be an else
        builder->CreateBr(then_blocks[i]);
      } else {
        Value *cond_v = codegen(cond);
        cond_v = load_if_is_lvalue(cond);
        if (i < n - 1) {
          builder->CreateCondBr(cond_v, then_blocks[i], cond_blocks[i + 1]);
        } else {
          builder->CreateCondBr(cond_v, then_blocks[i], merge_bb);
        }
      }

      /// then clause
      builder->SetInsertPoint(then_blocks[i]);
      codegen(p->get_branch(i));

      /// go to merge block if there is no terminator instruction at the end of then
      if (!builder->GetInsertBlock()->back().isTerminator()) {
        builder->CreateBr(merge_bb);
      }
    }

    /// emit merge block
    func->getBasicBlockList().push_back(merge_bb);
    builder->SetInsertPoint(merge_bb);
    return nullptr;
  }

  Value *codegen_member_access(MemberAccess *p) {
    auto *builder = _cs->_builder;

    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();

    auto *from = codegen(lhs);
    Value *ret;
    switch (p->_access_type) {
    case MemberAccess::MemberAccessBracket: {
      if (lhs->is_lvalue()) {
        from = builder->CreateLoad(from);
      }
      auto *rhs_val = codegen(rhs);
      if (rhs->is_lvalue()) {
        rhs_val = builder->CreateLoad(rhs_val);
      }
      ret = builder->CreateGEP(from, rhs_val, "bracket_access");
      break;
    }
    case MemberAccess::MemberAccessMemberVariable: {
      if (lhs->is_lvalue() && lhs->get_type()->is_pointer() && ((PointerType *)lhs->get_type())->get_pointee()) {
        /// auto dereference pointers
        from = builder->CreateLoad(from);
      }
      ret = builder->CreateStructGEP(from, (unsigned)p->_access_idx, "member_variable");
      break;
    }
    case MemberAccess::MemberAccessMemberFunction:
      ret = codegen(rhs);
      break;
    case MemberAccess::MemberAccessEnumValue: {
      str enum_name = ast_cast<Identifier>(lhs)->get_name();
      auto *enum_decl = ast_cast<EnumDecl>(_ctx->get_type_decl(enum_name));
      str element_name = ast_cast<Identifier>(rhs)->get_name();
      int64_t val = enum_decl->get_value(element_name);

      // TODO IMPORTANT: ret = CodegenIntegerLiteral(_cs, (uint64_t) val, enum_decl->get_type()->get_size_bits());
      break;
    }
    default:
      TAN_ASSERT(false);
    }

    return ret;
  }

  Value *codegen_ptr_deref(UnaryOperator *p) {
    auto *builder = _cs->_builder;

    auto *rhs = p->get_rhs();
    Value *val = codegen(rhs);
    TAN_ASSERT(val->getType()->isPointerTy());

    /// load only if the pointer itself is an lvalue, so that the value after deref is always an lvalue
    if (rhs->is_lvalue()) {
      val = builder->CreateLoad(val, "ptr_deref");
    }
    return val;
  }
};

CodeGenerator::CodeGenerator(CompilerSession *cs, ASTContext *ctx) { _impl = new CodeGeneratorImpl(cs, ctx); }

llvm::Value *CodeGenerator::codegen(ASTBase *p) { return _impl->codegen(p); }

CodeGenerator::~CodeGenerator() { delete _impl; }

static AllocaInst *create_block_alloca(BasicBlock *block, llvm::Type *type, size_t size, const str &name) {
  block = &block->getParent()->getEntryBlock();
  IRBuilder<> tmp_builder(block, block->begin());
  if (size <= 1) {
    return tmp_builder.CreateAlloca(type, nullptr, name);
  } else {
    return tmp_builder.CreateAlloca(type, tmp_builder.getInt32((unsigned)size), name);
  }
}

} // namespace tanlang
