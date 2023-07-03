#include "codegen/code_generator.h"
#include "ast/ast_base.h"
#include "ast/type.h"
#include "ast/expr.h"
#include "ast/stmt.h"
#include "ast/decl.h"
#include "ast/intrinsic.h"
#include "ast/default_value.h"
#include "compiler/compiler.h"
#include "llvm_api/llvm_include.h"

namespace tanlang {

void CodeGenerator::init(CompilationUnit *cu) {
  _cu = cu;
  _sm = cu->source_manager();
  _llvm_ctx = new LLVMContext();
  _builder = new IRBuilder<>(*_llvm_ctx);
  _module = new Module(_sm->get_filename(), *_llvm_ctx);
  _module->setDataLayout(_target_machine->createDataLayout());
  _module->setTargetTriple(_target_machine->getTargetTriple().str());
  auto opt_level = (llvm::CodeGenOpt::Level)Compiler::compile_config.opt_level;
  _target_machine->setOptLevel(opt_level);

  /// add_ctx the current debug info version into the module
  _module->addModuleFlag(Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
  _module->addModuleFlag(Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

  /// debug related
  _di_builder = new DIBuilder(*_module);
  _di_file = _di_builder->createFile(_sm->get_filename(), ".");
  _di_cu = _di_builder->createCompileUnit(llvm::dwarf::DW_LANG_C, _di_file, "tan compiler", false, "", 0);
  _di_scope = {_di_file};
}

CodeGenerator::CodeGenerator(TargetMachine *target_machine) : _target_machine(target_machine) {}

CodeGenerator::~CodeGenerator() {
  if (_di_builder)
    delete _di_builder;
  if (_module)
    delete _module;
  if (_builder)
    delete _builder;
  if (_llvm_ctx)
    delete _llvm_ctx;
}

llvm::Value *CodeGenerator::run_impl(CompilationUnit *cu) {
  auto *ret = cached_visit(cu->ast());

  _di_builder->finalize(); // do this before any pass

  run_passes();

  return ret;
}

llvm::Value *CodeGenerator::cached_visit(ASTBase *p) {
  auto it = _llvm_value_cache.find(p);
  if (it != _llvm_value_cache.end()) {
    return it->second;
  }
  set_current_debug_location(p);

  visit(p);

  return _llvm_value_cache[p];
}

void CodeGenerator::default_visit(ASTBase *) { TAN_ASSERT(false); }

void CodeGenerator::run_passes() {
  auto opt_level = _target_machine->getOptLevel();
  bool debug = opt_level == llvm::CodeGenOpt::Level::None;

  if (!debug) {
    // Create the analysis managers
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;

    // Create the new pass manager builder
    PassBuilder PB(_target_machine);

    // Register all the basic analyses with the managers
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    // Create the pass manager.
    // TODO: opt level
    // FIXME: string.tan failed if using optimization
    ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O2);
    // Optimize the IR
    MPM.run(*_module, MAM);
  }
}

void CodeGenerator::emit_to_file(const str &filename) {
  std::error_code ec;
  llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
  if (ec) {
    Error err("Could not open file: " + ec.message());
    err.raise();
  }
  PassManager emit_pass;
  auto file_type = llvm::CGFT_ObjectFile;
  if (_target_machine->addPassesToEmitFile(emit_pass, dest, nullptr, file_type)) {
    Error err("Target machine can't emit a file of this type");
    err.raise();
  }
  emit_pass.run(*_module);
  dest.flush();
}

void CodeGenerator::dump_ir() const { _module->print(llvm::outs(), nullptr); }

// ===================================================

AllocaInst *CodeGenerator::create_block_alloca(BasicBlock *block, llvm::Type *type, size_t size, const str &name) {
  block = &block->getParent()->getEntryBlock();
  IRBuilder<> tmp_builder(block, block->begin());
  if (size <= 1) {
    return tmp_builder.CreateAlloca(type, nullptr, name);
  } else {
    return tmp_builder.CreateAlloca(type, tmp_builder.getInt32((unsigned)size), name);
  }
}

llvm::Value *CodeGenerator::convert_llvm_type_to(Expr *expr, Type *dest) {
  /// load if lvalue
  Value *loaded = load_if_is_lvalue(expr);

  Type *orig = expr->get_type();

  bool is_pointer1 = orig->is_pointer();
  bool is_pointer2 = dest->is_pointer();

  /// early return if types are the same
  if (orig == dest) {
    return loaded;
  };
  if (is_pointer1 && is_pointer2) {
    /// cast between pointer types (including pointers to pointers)
    return _builder->CreateBitCast(loaded, to_llvm_type(dest));
  } else if ((orig->is_int() || orig->is_char()) && (dest->is_char() || dest->is_int())) { /// between int
    if (dest->is_unsigned())
      return _builder->CreateZExtOrTrunc(loaded, to_llvm_type(dest));
    else
      return _builder->CreateSExtOrTrunc(loaded, to_llvm_type(dest));
  } else if (orig->is_int() && dest->is_float()) { /// int to float/double
    if (orig->is_unsigned()) {
      return _builder->CreateUIToFP(loaded, to_llvm_type(dest));
    } else {
      return _builder->CreateSIToFP(loaded, to_llvm_type(dest));
    }
  } else if (orig->is_float() && dest->is_int()) { /// float/double to int
    if (dest->is_unsigned()) {
      return _builder->CreateFPToUI(loaded, to_llvm_type(dest));
    } else {
      return _builder->CreateFPToSI(loaded, to_llvm_type(dest));
    }
  } else if (orig->is_float() && dest->is_float()) { /// float <-> double
    return _builder->CreateFPCast(loaded, to_llvm_type(dest));
  } else if (orig->is_bool() && dest->is_int()) {    /// bool to int
    return _builder->CreateZExtOrTrunc(loaded, to_llvm_type(dest));
  } else if (orig->is_bool() && dest->is_float()) {  /// bool to float
    return _builder->CreateUIToFP(loaded, to_llvm_type(dest));
  } else if (dest->is_bool()) {
    if (orig->is_float()) { /// float to bool
      if (orig->get_size_bits() == 32) {
        return _builder->CreateFCmpONE(loaded, ConstantFP::get(_builder->getFloatTy(), 0.0f));
      } else {
        return _builder->CreateFCmpONE(loaded, ConstantFP::get(_builder->getDoubleTy(), 0.0f));
      }
    } else if (orig->is_pointer()) { /// pointer to bool
      size_t s1 = _target_machine->getPointerSizeInBits(0);
      loaded = _builder->CreatePtrToInt(loaded, _builder->getIntNTy((unsigned)s1));
      return _builder->CreateICmpNE(loaded, ConstantInt::get(_builder->getIntNTy((unsigned)s1), 0, false));
    } else if (orig->is_int()) { /// int to bool
      return _builder->CreateICmpNE(loaded,
                                    ConstantInt::get(_builder->getIntNTy((unsigned)orig->get_size_bits()), 0, false));
    }
  } else if (orig->is_string() && dest->is_pointer()) { /// string to pointer, don't need to do anything
    return loaded;
  } else if (orig->is_array() && dest->is_pointer()) {  /// array to pointer, don't need to do anything
    return loaded;
  } else if (orig->is_array() && dest->is_string()) {   /// array to string, don't need to do anything
    return loaded;
  }

  error(ErrorType::SEMANTIC_ERROR, expr, "Cannot perform type conversion");
}

llvm::Value *CodeGenerator::load_if_is_lvalue(Expr *expr) {
  Value *val = _llvm_value_cache[expr];
  TAN_ASSERT(val);

  if (expr->is_lvalue()) {
    return _builder->CreateLoad(to_llvm_type(expr->get_type()), val, "lvalue_load");
  }
  return val;
}

llvm::Type *CodeGenerator::to_llvm_type(Type *p) {
  TAN_ASSERT(p);
  TAN_ASSERT(!p->is_ref());

  auto it = _llvm_type_cache.find(p);
  if (it != _llvm_type_cache.end()) {
    return it->second;
  }

  llvm::Type *ret = nullptr;

  if (p->is_primitive()) { /// primitive types
    int size_bits = p->get_size_bits();
    if (p->is_int()) {
      ret = _builder->getIntNTy((unsigned)size_bits);
    } else if (p->is_char()) {
      ret = _builder->getInt8Ty();
    } else if (p->is_bool()) {
      ret = _builder->getInt1Ty();
    } else if (p->is_float()) {
      if (32 == size_bits) {
        ret = _builder->getFloatTy();
      } else if (64 == size_bits) {
        ret = _builder->getDoubleTy();
      } else {
        TAN_ASSERT(false);
      }
    } else if (p->is_void()) {
      ret = _builder->getVoidTy();
    }
  } else if (p->is_string()) { /// str as char*
    ret = _builder->getInt8PtrTy();
  } else if (p->is_struct()) { /// struct
    // avoid infinite recursion
    _llvm_type_cache[p] = ret = llvm::StructType::create(*_llvm_ctx, p->get_typename());
    auto types = ((StructType *)p)->get_member_types();
    vector<llvm::Type *> elements(types.size(), nullptr);
    for (size_t i = 0; i < types.size(); ++i) {
      elements[i] = to_llvm_type(types[i]);
    }
    ((llvm::StructType *)ret)->setBody(elements);
  } else if (p->is_array()) { /// array as pointer
    auto *e_type = to_llvm_type(((ArrayType *)p)->get_element_type());
    ret = e_type->getPointerTo();
  } else if (p->is_pointer()) { /// pointer
    auto *e_type = to_llvm_type(((PointerType *)p)->get_pointee());
    ret = e_type->getPointerTo();
  } else if (p->is_function()) {
    auto *func_type = (tanlang::FunctionType *)p;
    vector<llvm::Type *> arg_types{};
    for (auto *t : func_type->get_arg_types()) {
      arg_types.push_back(to_llvm_type(t));
    }
    auto *ret_type = to_llvm_type(func_type->get_return_type());
    ret = llvm::FunctionType::get(ret_type, arg_types, false);
  } else {
    TAN_ASSERT(false);
  }

  _llvm_type_cache[p] = ret;
  return ret;
}

llvm::Metadata *CodeGenerator::to_llvm_metadata(Type *p, uint32_t loc) {
  TAN_ASSERT(p);
  TAN_ASSERT(!p->is_ref());

  auto it = _llvm_meta_cache.find(p);
  if (it != _llvm_meta_cache.end()) {
    return it->second;
  }

  DIType *ret = nullptr;
  if (p->is_primitive()) { /// primitive types
    unsigned dwarf_encoding = 0;
    int size_bits = p->get_size_bits();
    if (p->is_int()) {
      if (p->is_unsigned()) {
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

    ret = _di_builder->createBasicType(p->get_typename(), (uint64_t)size_bits, dwarf_encoding);
  } else if (p->is_string()) { /// str as char*
    auto *e_di_type = _di_builder->createBasicType("u8", 8, llvm::dwarf::DW_ATE_unsigned_char);
    ret = _di_builder->createPointerType(e_di_type, _target_machine->getPointerSizeInBits(0),
                                         (unsigned)_target_machine->getPointerSizeInBits(0), llvm::None,
                                         p->get_typename());
  } else if (p->is_struct()) { /// struct
    auto member_types = ((StructType *)p)->get_member_types();
    unsigned n = (unsigned)member_types.size();

    // avoid infinite recursion by inserting a placeholder
    ret = _di_builder->createStructType(
        get_current_di_scope(), p->get_typename(), _di_file, (unsigned)_sm->get_line(loc), (uint32_t)p->get_size_bits(),
        (uint32_t)p->get_align_bits(), DINode::DIFlags::FlagZero, nullptr,
        _di_builder->getOrCreateArray(vector<Metadata *>(n, nullptr)), 0, nullptr, p->get_typename());

    vector<Metadata *> elements(member_types.size(), nullptr);
    for (unsigned i = 0; i < n; ++i) {
      elements[i] = to_llvm_metadata(member_types[i], loc);
    }
    // work around replaceElements()'s check
    ret->replaceOperandWith(4, _di_builder->getOrCreateArray(elements).get());
  } else if (p->is_array()) { /// array as pointer
    auto *sub = to_llvm_metadata(((ArrayType *)p)->get_element_type(), loc);
    ret = _di_builder->createPointerType((DIType *)sub, _target_machine->getPointerSizeInBits(0),
                                         (unsigned)_target_machine->getPointerSizeInBits(0), llvm::None,
                                         p->get_typename());
  } else if (p->is_pointer()) { /// pointer
    // avoid infinite recursion by inserting a placeholder
    _llvm_meta_cache[p] = ret = _di_builder->createPointerType(nullptr, _target_machine->getPointerSizeInBits(0),
                                                               (unsigned)_target_machine->getPointerSizeInBits(0),
                                                               llvm::None, p->get_typename());
    auto *sub = to_llvm_metadata(((PointerType *)p)->get_pointee(), loc);
    ret->replaceOperandWith(3, sub);
  } else {
    TAN_ASSERT(false);
  }

  return _llvm_meta_cache[p] = ret;
}

llvm::DISubroutineType *CodeGenerator::create_function_debug_info_type(llvm::Metadata *ret,
                                                                       vector<llvm::Metadata *> args) {
  vector<Metadata *> types{ret};
  types.reserve(args.size());
  types.insert(types.begin() + 1, args.begin(), args.end());
  // return _di_builder->createSubroutineType(_di_builder->getOrCreateTypeArray(types), DINode::FlagZero,
  //                                          llvm::dwarf::DW_CC_normal);
  return _di_builder->createSubroutineType(_di_builder->getOrCreateTypeArray(types));
}

void CodeGenerator::set_current_debug_location(ASTBase *p) {
  unsigned line = _sm->get_line(p->start()) + 1;
  unsigned col = _sm->get_col(p->start()) + 1;
  _builder->SetCurrentDebugLocation(DILocation::get(*_llvm_ctx, line, col, this->get_current_di_scope()));
}

DIScope *CodeGenerator::get_current_di_scope() const { return _di_scope.back(); }
void CodeGenerator::push_di_scope(DIScope *scope) { _di_scope.push_back(scope); }
void CodeGenerator::pop_di_scope() { _di_scope.pop_back(); }

DebugLoc CodeGenerator::debug_loc_of_node(ASTBase *p, MDNode *scope) {
  return DILocation::get(*_llvm_ctx, _sm->get_line(p->start()), _sm->get_col(p->start()), scope);
}

// ===================================================

Value *CodeGenerator::codegen_var_arg_decl(Decl *p) {
  llvm::Type *type = to_llvm_type(p->get_type());
  auto *ret = create_block_alloca(_builder->GetInsertBlock(), type, 1, p->get_name());

  // default value of var declaration
  if (p->get_node_type() == ASTNodeType::VAR_DECL) {
    Value *default_value = codegen_type_default_value(p->get_type());
    TAN_ASSERT(default_value);
    _builder->CreateStore(default_value, ret);
  }

  // debug info
  auto *curr_di_scope = get_current_di_scope();
  auto *arg_meta = to_llvm_metadata(p->get_type(), p->start());
  auto *di_arg = _di_builder->createAutoVariable(curr_di_scope, p->get_name(), _di_file, _sm->get_line(p->start()),
                                                 (DIType *)arg_meta);
  _di_builder->insertDeclare(ret, di_arg, _di_builder->createExpression(), debug_loc_of_node(p, curr_di_scope),
                             _builder->GetInsertBlock());
  return ret;
}

Value *CodeGenerator::codegen_struct_default_value(StructType *ty) {
  StructDecl *struct_decl = ty->get_decl();

  auto member_types = ty->get_member_types();
  TAN_ASSERT(member_types.size() == struct_decl->get_member_decls().size());

  vector<Constant *> values(member_types.size(), nullptr);
  for (size_t i = 0; i < member_types.size(); ++i) {
    Expr *v = struct_decl->get_member_default_val((int)i);

    if (v) {
      TAN_ASSERT(v->is_comptime_known());
      // default value is set in the struct definition
      values[i] = (llvm::Constant *)cached_visit(v);
    } else {
      values[i] = (llvm::Constant *)codegen_type_default_value(member_types[i]);
    }
  }

  return ConstantStruct::get((llvm::StructType *)to_llvm_type(ty), values);
}

Value *CodeGenerator::codegen_type_default_value(Type *p) {
  TAN_ASSERT(!p->is_ref());

  Value *ret = nullptr;
  if (p->is_primitive() || p->is_string() || p->is_array() || p->is_pointer()) {
    ret = cached_visit(DefaultValue::CreateTypeDefaultValueLiteral(_sm->src(), p));

  } else if (p->is_struct()) {
    ret = codegen_struct_default_value((StructType *)p);
  } else {
    TAN_ASSERT(false);
  }

  return ret;
}

Value *CodeGenerator::codegen_literals(Literal *p) {
  llvm::Type *type = to_llvm_type(p->get_type());
  Value *ret = nullptr;
  Type *ptype = p->get_type();
  if (ptype->is_primitive()) { /// primitive types
    int size_bits = ptype->get_size_bits();

    if (ptype->is_char()) { // NOTE: must be before is_int() check because char is technically an integer
      ret = ConstantInt::get(type, pcast<CharLiteral>(p)->get_value());
    } else if (ptype->is_int()) {
      auto pp = pcast<IntegerLiteral>(p);
      ret = ConstantInt::get(_builder->getIntNTy((unsigned)size_bits), pp->get_value(), !pp->is_unsigned());
    } else if (ptype->is_bool()) {
      auto pp = pcast<BoolLiteral>(p);
      ret = ConstantInt::get(type, (uint64_t)pp->get_value());
    } else if (ptype->is_float()) {
      ret = ConstantFP::get(type, pcast<FloatLiteral>(p)->get_value());
    } else {
      TAN_ASSERT(false);
    }
  } else if (ptype->is_string()) { /// str as char*
    ret = _builder->CreateGlobalStringPtr(pcast<StringLiteral>(p)->get_value());
  } else if (ptype->is_struct()) { /// struct
    // TODO: Implement struct literal
    TAN_ASSERT(false);
  } else if (ptype->is_array()) { /// array as pointer
    auto arr = pcast<ArrayLiteral>(p);

    /// element type
    auto elements = arr->get_elements();
    auto *e_type = to_llvm_type(((ArrayType *)ptype)->get_element_type());

    /// codegen element values
    size_t n = elements.size();
    ret = create_block_alloca(_builder->GetInsertBlock(), e_type, n, "array_storage");
    for (size_t i = 0; i < n; ++i) {
      auto *idx = _builder->getInt32((unsigned)i);
      auto *e_val = cached_visit(elements[i]);
      auto *e_ptr = _builder->CreateGEP(e_type, ret, idx);
      _builder->CreateStore(e_val, e_ptr);
    }
  } else if (ptype->is_pointer()) { /// the pointer literal is nullptr
    ret = ConstantPointerNull::get((llvm::PointerType *)type);
  } else {
    TAN_ASSERT(false);
  }

  return ret;
}

Value *CodeGenerator::codegen_func_prototype(FunctionDecl *p, bool import_) {
  auto linkage = Function::InternalLinkage;
  if (p->is_external()) {
    linkage = Function::ExternalWeakLinkage;
  }
  if (p->is_public()) {
    if (import_) {
      linkage = Function::ExternalWeakLinkage;
    } else {
      linkage = Function::ExternalLinkage;
    }
  }
  Function *func = Function::Create((llvm::FunctionType *)to_llvm_type(p->get_type()), linkage, p->get_name(), _module);
  func->setCallingConv(llvm::CallingConv::C);
  return func;
}

Value *CodeGenerator::codegen_ptr_deref(UnaryOperator *p) {
  auto *rhs = p->get_rhs();
  Value *val = cached_visit(rhs);
  TAN_ASSERT(val->getType()->isPointerTy());

  /// load only if the pointer itself is an lvalue, so that the value after deref is always an lvalue
  if (rhs->is_lvalue()) {
    val = _builder->CreateLoad(to_llvm_type(rhs->get_type()), val, "ptr_deref");
  }
  return val;
}

Value *CodeGenerator::codegen_relop(BinaryOperator *p) {
  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  Value *l = cached_visit(lhs);
  Value *r = cached_visit(rhs);

  r = load_if_is_lvalue(rhs);
  l = load_if_is_lvalue(lhs);

  Value *ret = nullptr;
  switch (p->get_op()) {
  case BinaryOpKind::BAND:
    ret = _builder->CreateAnd(l, r, "binary_and");
    break;
  case BinaryOpKind::LAND:
    ret = _builder->CreateAnd(l, r, "logical_and");
    break;
  case BinaryOpKind::BOR:
    ret = _builder->CreateOr(l, r, "binary_or");
    break;
  case BinaryOpKind::LOR:
    ret = _builder->CreateOr(l, r, "logical_or");
    break;
  case BinaryOpKind::XOR:
    ret = _builder->CreateXor(l, r, "logical_or");
    break;
  default:
    TAN_ASSERT(false);
    break;
  }

  TAN_ASSERT(ret);
  return ret;
}

Value *CodeGenerator::codegen_bnot(UnaryOperator *p) {
  auto *rhs = cached_visit(p->get_rhs());
  if (!rhs) {
    error(ErrorType::SEMANTIC_ERROR, p, "Invalid operand");
  }
  if (p->get_rhs()->is_lvalue()) {
    rhs = _builder->CreateLoad(to_llvm_type(p->get_rhs()->get_type()), rhs);
  }
  return _builder->CreateNot(rhs);
}

Value *CodeGenerator::codegen_lnot(UnaryOperator *p) {
  auto *rhs = cached_visit(p->get_rhs());

  if (!rhs) {
    error(ErrorType::SEMANTIC_ERROR, p, "Invalid operand");
  }

  if (p->get_rhs()->is_lvalue()) {
    rhs = _builder->CreateLoad(to_llvm_type(p->get_rhs()->get_type()), rhs);
  }
  /// get value size in bits
  auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
  if (rhs->getType()->isFloatingPointTy()) {
    return _builder->CreateFCmpOEQ(rhs, ConstantFP::get(_builder->getFloatTy(), 0.0f));
  } else if (rhs->getType()->isSingleValueType()) {
    return _builder->CreateICmpEQ(rhs, ConstantInt::get(_builder->getIntNTy((unsigned)size_in_bits), 0, false));
  }

  error(ErrorType::SEMANTIC_ERROR, p, "Invalid operand");
}

Value *CodeGenerator::codegen_address_of(UnaryOperator *p) {
  if (!p->get_rhs()->is_lvalue()) {
    error(ErrorType::SEMANTIC_ERROR, p, "Cannot get address of rvalue");
  }

  return cached_visit(p->get_rhs());
}

Value *CodeGenerator::codegen_arithmetic(BinaryOperator *p) {
  /// binary operator
  auto *lhs = p->get_lhs();
  auto *rhs = p->get_rhs();
  Value *l = cached_visit(lhs);
  Value *r = cached_visit(rhs);
  r = load_if_is_lvalue(rhs);
  l = load_if_is_lvalue(lhs);

  Value *ret = nullptr;
  if (l->getType()->isFloatingPointTy()) {
    /// float arithmetic
    switch (p->get_op()) {
    case BinaryOpKind::MULTIPLY:
      ret = _builder->CreateFMul(l, r, "mul_tmp");
      break;
    case BinaryOpKind::DIVIDE:
      ret = _builder->CreateFDiv(l, r, "div_tmp");
      break;
    case BinaryOpKind::SUM:
      ret = _builder->CreateFAdd(l, r, "sum_tmp");
      break;
    case BinaryOpKind::SUBTRACT:
      ret = _builder->CreateFSub(l, r, "sub_tmp");
      break;
    case BinaryOpKind::MOD:
      ret = _builder->CreateFRem(l, r, "mod_tmp");
      break;
    default:
      TAN_ASSERT(false);
      break;
    }
  } else {
    /// integer arithmetic
    switch (p->get_op()) {
    case BinaryOpKind::MULTIPLY:
      ret = _builder->CreateMul(l, r, "mul_tmp");
      break;
    case BinaryOpKind::DIVIDE: {
      auto ty = lhs->get_type();
      if (ty->is_unsigned()) {
        ret = _builder->CreateUDiv(l, r, "div_tmp");
      } else {
        ret = _builder->CreateSDiv(l, r, "div_tmp");
      }
      break;
    }
    case BinaryOpKind::SUM:
      ret = _builder->CreateAdd(l, r, "sum_tmp");
      break;
    case BinaryOpKind::SUBTRACT:
      ret = _builder->CreateSub(l, r, "sub_tmp");
      break;
    case BinaryOpKind::MOD: {
      auto ty = lhs->get_type();
      if (ty->is_unsigned()) {
        ret = _builder->CreateURem(l, r, "mod_tmp");
      } else {
        ret = _builder->CreateSRem(l, r, "mod_tmp");
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

Value *CodeGenerator::codegen_comparison(BinaryOperator *p) {
  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  Value *l = cached_visit(lhs);
  Value *r = cached_visit(rhs);

  bool is_signed = !lhs->get_type()->is_unsigned();
  r = load_if_is_lvalue(rhs);
  l = load_if_is_lvalue(lhs);

  Value *ret = nullptr;
  if (l->getType()->isFloatingPointTy()) {
    switch (p->get_op()) {
    case BinaryOpKind::EQ:
      ret = _builder->CreateFCmpOEQ(l, r, "eq");
      break;
    case BinaryOpKind::NE:
      ret = _builder->CreateFCmpONE(l, r, "ne");
      break;
    case BinaryOpKind::GT:
      ret = _builder->CreateFCmpOGT(l, r, "gt");
      break;
    case BinaryOpKind::GE:
      ret = _builder->CreateFCmpOGE(l, r, "ge");
      break;
    case BinaryOpKind::LT:
      ret = _builder->CreateFCmpOLT(l, r, "lt");
      break;
    case BinaryOpKind::LE:
      ret = _builder->CreateFCmpOLE(l, r, "le");
      break;
    default:
      TAN_ASSERT(false);
      break;
    }
  } else {
    switch (p->get_op()) {
    case BinaryOpKind::EQ:
      ret = _builder->CreateICmpEQ(l, r, "eq");
      break;
    case BinaryOpKind::NE:
      ret = _builder->CreateICmpNE(l, r, "ne");
      break;
    case BinaryOpKind::GT:
      if (is_signed) {
        ret = _builder->CreateICmpSGT(l, r, "gt");
      } else {
        ret = _builder->CreateICmpUGT(l, r, "gt");
      }
      break;
    case BinaryOpKind::GE:
      if (is_signed) {
        ret = _builder->CreateICmpSGE(l, r, "ge");
      } else {
        ret = _builder->CreateICmpUGE(l, r, "ge");
      }
      break;
    case BinaryOpKind::LT:
      if (is_signed) {
        ret = _builder->CreateICmpSLT(l, r, "lt");
      } else {
        ret = _builder->CreateICmpULT(l, r, "lt");
      }
      break;
    case BinaryOpKind::LE:
      if (is_signed) {
        ret = _builder->CreateICmpSLE(l, r, "le");
      } else {
        ret = _builder->CreateICmpULE(l, r, "le");
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

Value *CodeGenerator::codegen_member_access(BinaryOperator *_p) {
  auto *p = pcast<MemberAccess>(_p);
  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();

  auto *lhs_val = cached_visit(lhs);

  Value *ret = nullptr;
  switch (p->_access_type) {
  case MemberAccess::MemberAccessBracket: {
    lhs_val = load_if_is_lvalue(lhs);

    cached_visit(rhs);
    auto *rhs_val = load_if_is_lvalue(rhs);

    llvm::Type *element_type = nullptr;
    if (lhs->get_type()->is_array()) { /// array
      auto *lhs_type = (tanlang::ArrayType *)lhs->get_type();
      element_type = to_llvm_type(lhs_type->get_element_type());
    } else if (lhs->get_type()->is_string()) {  /// string
      element_type = llvm::Type::getInt8Ty(*_llvm_ctx);
    } else if (lhs->get_type()->is_pointer()) { /// pointer
      auto *lhs_type = (tanlang::PointerType *)lhs->get_type();
      element_type = to_llvm_type(lhs_type->get_pointee());
    } else {
      TAN_ASSERT(false);
    }
    ret = _builder->CreateGEP(element_type, lhs_val, rhs_val, "bracket_access");
    break;
  }
  case MemberAccess::MemberAccessMemberVariable: {
    StructType *st = nullptr;
    if (lhs->get_type()->is_pointer()) { /// auto dereference pointers
      lhs_val = load_if_is_lvalue(lhs);
      st = (StructType *)((PointerType *)lhs->get_type())->get_pointee();
    } else {
      st = (StructType *)lhs->get_type();
    }
    TAN_ASSERT(st->is_struct());
    TAN_ASSERT(lhs_val->getType()->isPointerTy());

    lhs_val->getType()->print(llvm::outs());
    ret = _builder->CreateStructGEP(to_llvm_type(st), lhs_val, (unsigned)p->_access_idx, "member_variable");
    break;
  }
  case MemberAccess::MemberAccessMemberFunction:
    // TODO: codegen for member function call
    ret = cached_visit(rhs);
    break;
  default:
    TAN_ASSERT(false);
  }

  return ret;
}

// ===================================================

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Program) {
  for (auto *c : p->get_children()) {
    cached_visit(c);
  }
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Identifier) {
  switch (p->get_id_type()) {
  case IdentifierType::ID_VAR_REF:
    _llvm_value_cache[p] = cached_visit(p->get_var_ref());
    break;
  case IdentifierType::ID_TYPE_REF:
  default:
    TAN_ASSERT(false);
    break;
  }
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Parenthesis) { _llvm_value_cache[p] = cached_visit(p->get_sub()); }

DEFINE_AST_VISITOR_IMPL(CodeGenerator, If) {
  Function *func = _builder->GetInsertBlock()->getParent();
  size_t n = p->get_num_branches();

  /// create basic blocks
  vector<BasicBlock *> cond_blocks(n);
  vector<BasicBlock *> then_blocks(n);
  for (size_t i = 0; i < n; ++i) {
    cond_blocks[i] = BasicBlock::Create(*_llvm_ctx, "cond", func);
    then_blocks[i] = BasicBlock::Create(*_llvm_ctx, "branch", func);
  }
  BasicBlock *merge_bb = BasicBlock::Create(*_llvm_ctx, "endif", func);

  /// codegen branches
  _builder->CreateBr(cond_blocks[0]);
  for (size_t i = 0; i < n; ++i) {
    /// condition
    _builder->SetInsertPoint(cond_blocks[i]);

    Expr *cond = p->get_predicate(i);
    if (!cond) {              /// else clause, immediately go to then block
      TAN_ASSERT(i == n - 1); /// only the last branch can be an else
      _builder->CreateBr(then_blocks[i]);
    } else {
      cached_visit(cond);
      Value *cond_v = load_if_is_lvalue(cond);
      if (i < n - 1) {
        _builder->CreateCondBr(cond_v, then_blocks[i], cond_blocks[i + 1]);
      } else {
        _builder->CreateCondBr(cond_v, then_blocks[i], merge_bb);
      }
    }

    /// then clause
    _builder->SetInsertPoint(then_blocks[i]);
    cached_visit(p->get_branch(i));

    /// go to merge block if there is no terminator instruction at the end of then
    if (!_builder->GetInsertBlock()->back().isTerminator()) {
      _builder->CreateBr(merge_bb);
    }
  }

  /// emit merge block
  _builder->SetInsertPoint(merge_bb);

  _llvm_value_cache[p] = nullptr;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, VarDecl) { _llvm_value_cache[p] = codegen_var_arg_decl(p); }

DEFINE_AST_VISITOR_IMPL(CodeGenerator, ArgDecl) { _llvm_value_cache[p] = codegen_var_arg_decl(p); }

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Return) {
  auto rhs = p->get_rhs();
  if (rhs) { /// return with value
    Value *result = cached_visit(rhs);
    if (rhs->is_lvalue()) {
      result = _builder->CreateLoad(to_llvm_type(rhs->get_type()), result);
    }
    _builder->CreateRet(result);
  } else { /// return void
    _builder->CreateRetVoid();
  }
  _llvm_value_cache[p] = nullptr;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, CompoundStmt) {
  for (auto *c : p->get_children()) {
    cached_visit(c);
  }
  _llvm_value_cache[p] = nullptr;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, BinaryOrUnary) { _llvm_value_cache[p] = cached_visit(p->get_expr_ptr()); }

DEFINE_AST_VISITOR_IMPL(CodeGenerator, BinaryOperator) {
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
    ret = codegen_member_access(p);
    break;
  default:
    TAN_ASSERT(false);
    break;
  }

  _llvm_value_cache[p] = ret;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, UnaryOperator) {
  Value *ret = nullptr;

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
    ret = cached_visit(rhs);
    break;
  case UnaryOpKind::MINUS: {
    auto *r = cached_visit(rhs);
    if (rhs->is_lvalue()) {
      r = _builder->CreateLoad(to_llvm_type(rhs->get_type()), r);
    }
    if (r->getType()->isFloatingPointTy()) {
      ret = _builder->CreateFNeg(r);
    } else {
      ret = _builder->CreateNeg(r);
    }
    break;
  }
  default:
    TAN_ASSERT(false);
    break;
  }

  _llvm_value_cache[p] = ret;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Cast) {
  auto lhs = p->get_lhs();
  auto *dest_type = to_llvm_type(p->get_type());

  Value *val = cached_visit(lhs);
  TAN_ASSERT(val);

  Value *ret = nullptr;
  val = convert_llvm_type_to(lhs, p->get_type()); // lvalue will be loaded here
  if (lhs->is_lvalue()) {
    ret = create_block_alloca(_builder->GetInsertBlock(), dest_type, 1, "casted");
    _builder->CreateStore(val, ret);
  } else {
    ret = val;
  }

  _llvm_value_cache[p] = ret;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Assignment) {
  /// codegen the lhs and rhs
  auto *lhs = pcast<Expr>(p->get_lhs());
  auto *rhs = p->get_rhs();

  // type of lhs is the same as type of the assignment
  if (!lhs->is_lvalue()) {
    error(ErrorType::SEMANTIC_ERROR, lhs, "Value can only be assigned to an lvalue");
  }

  Value *from = cached_visit(rhs);
  from = load_if_is_lvalue(rhs);
  Value *to = cached_visit(lhs);
  TAN_ASSERT(from && to);

  _builder->CreateStore(from, to);

  _llvm_value_cache[p] = to;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, FunctionCall) {
  FunctionDecl *callee = p->_callee;
  auto *callee_type = (tanlang::FunctionType *)callee->get_type();
  size_t n = callee->get_n_args();

  // args
  vector<Value *> arg_vals;
  for (size_t i = 0; i < n; ++i) {
    auto actual_arg = p->_args[i];
    auto *a = cached_visit(actual_arg);
    if (!a) {
      error(ErrorType::SEMANTIC_ERROR, actual_arg, "Invalid function call argument");
    }

    // implicit cast
    auto expected_ty = callee_type->get_arg_types()[i];
    a = convert_llvm_type_to(actual_arg, expected_ty);
    arg_vals.push_back(a);
  }

  auto *func_type = (llvm::FunctionType *)to_llvm_type(callee->get_type());
  auto *F = cached_visit(callee);

  _llvm_value_cache[p] = _builder->CreateCall(func_type, F, arg_vals);
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, FunctionDecl) {
  auto *func_type = (tanlang::FunctionType *)p->get_type();

  auto ret_ty = func_type->get_return_type();
  Metadata *ret_meta = to_llvm_metadata(ret_ty, p->start());

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
    auto ty = func_type->get_arg_types()[i];
    arg_metas.push_back(to_llvm_metadata(ty, p->start()));
  }

  /// function implementation
  if (!p->is_external()) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*_llvm_ctx, "func_entry", F);
    _builder->SetInsertPoint(main_block);

    /// debug information
    DIScope *di_scope = get_current_di_scope();
    auto *di_func_t = create_function_debug_info_type(ret_meta, arg_metas);
    DISubprogram *subprogram = _di_builder->createFunction(
        di_scope, func_name, func_name, _di_file, _sm->get_line(p->start()), di_func_t, _sm->get_col(p->start()),
        DINode::FlagPrototyped, DISubprogram::SPFlagDefinition, nullptr, nullptr, nullptr);
    F->setSubprogram(subprogram);
    push_di_scope(subprogram);

    /// add_ctx all function arguments to scope
    size_t i = 0;
    for (auto &a : F->args()) {
      auto arg_name = p->get_arg_name(i);
      auto *arg_val = cached_visit(p->get_arg_decls()[i]);
      _builder->CreateStore(&a, arg_val);

      /// create a debug descriptor for the arguments
      auto *arg_meta = to_llvm_metadata(func_type->get_arg_types()[i], p->start());
      llvm::DILocalVariable *di_arg = _di_builder->createParameterVariable(
          subprogram, arg_name, (unsigned)i + 1, _di_file, _sm->get_line(p->start()), (DIType *)arg_meta, true);
      _di_builder->insertDeclare(arg_val, di_arg, _di_builder->createExpression(),
                                 debug_loc_of_node(p->get_arg_decls()[i], subprogram), _builder->GetInsertBlock());
      ++i;
    }

    /// generate function body
    cached_visit(p->get_body());

    /// create a return instruction if there is none, the return value is the default value of the return type
    auto *trailing_block = _builder->GetInsertBlock();
    if (trailing_block->sizeWithoutDebug() == 0 || !trailing_block->back().isTerminator()) {
      if (ret_ty->is_void()) {
        _builder->CreateRetVoid();
      } else {
        auto *ret_val = codegen_type_default_value(ret_ty);
        TAN_ASSERT(ret_val);
        _builder->CreateRet(ret_val);
      }
    }
    pop_di_scope();
  }

  _llvm_value_cache[p] = F;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Import) {
  for (FunctionDecl *f : p->get_imported_funcs()) {
    /// do nothing for already defined intrinsics
    auto *func = _module->getFunction(f->get_name());
    if (func) {
      _llvm_value_cache[f] = func;
    }
    _llvm_value_cache[f] = codegen_func_prototype(f);
  }

  _llvm_value_cache[p] = nullptr;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Intrinsic) {
  Value *ret = nullptr;
  switch (p->get_intrinsic_type()) {
    /// trivial codegen
  case IntrinsicType::GET_DECL:
  case IntrinsicType::LINENO:
  case IntrinsicType::ABORT:
  case IntrinsicType::STACK_TRACE:
  case IntrinsicType::FILENAME: {
    ret = cached_visit(p->get_sub());
    break;
  }
  case IntrinsicType::NOOP:
  default:
    break;
  }

  _llvm_value_cache[p] = ret;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, ArrayLiteral) { _llvm_value_cache[p] = codegen_literals(p); }
DEFINE_AST_VISITOR_IMPL(CodeGenerator, CharLiteral) { _llvm_value_cache[p] = codegen_literals(p); }
DEFINE_AST_VISITOR_IMPL(CodeGenerator, BoolLiteral) { _llvm_value_cache[p] = codegen_literals(p); }
DEFINE_AST_VISITOR_IMPL(CodeGenerator, IntegerLiteral) { _llvm_value_cache[p] = codegen_literals(p); }
DEFINE_AST_VISITOR_IMPL(CodeGenerator, FloatLiteral) { _llvm_value_cache[p] = codegen_literals(p); }
DEFINE_AST_VISITOR_IMPL(CodeGenerator, StringLiteral) { _llvm_value_cache[p] = codegen_literals(p); }
DEFINE_AST_VISITOR_IMPL(CodeGenerator, NullPointerLiteral) { _llvm_value_cache[p] = codegen_literals(p); }

DEFINE_AST_VISITOR_IMPL(CodeGenerator, StructDecl) {
  // don't do anything
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, Loop) {
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

    Function *func = _builder->GetInsertBlock()->getParent();

    /// make sure to set _loop_start and _loop_end before generating loop_body, cuz break and continue statements
    /// use these two (get_loop_start() and get_loop_end())
    p->_loop_start = BasicBlock::Create(*_llvm_ctx, "loop", func);
    BasicBlock *loop_body = BasicBlock::Create(*_llvm_ctx, "loop_body", func);
    p->_loop_end = BasicBlock::Create(*_llvm_ctx, "after_loop", func);

    /// start loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!_builder->GetInsertBlock()->back().isTerminator()) {
      _builder->CreateBr(p->_loop_start);
    }

    /// condition
    _builder->SetInsertPoint(p->_loop_start);
    auto *cond = cached_visit(p->get_predicate());
    if (!cond) {
      error(ErrorType::SEMANTIC_ERROR, p, "Expected a condition expression");
    }
    _builder->CreateCondBr(cond, loop_body, p->_loop_end);

    /// loop body
    _builder->SetInsertPoint(loop_body);
    cached_visit(p->get_body());

    /// go back to the start of the loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!_builder->GetInsertBlock()->back().isTerminator()) {
      _builder->CreateBr(p->_loop_start);
    }

    /// end loop
    _builder->SetInsertPoint(p->_loop_end);
  } else {
    TAN_ASSERT(false);
  }

  _llvm_value_cache[p] = nullptr;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, BreakContinue) {
  auto loop = p->get_parent_loop();
  TAN_ASSERT(loop);

  auto s = loop->_loop_start;
  auto e = loop->_loop_end;
  TAN_ASSERT(s);
  TAN_ASSERT(e);

  if (p->get_node_type() == ASTNodeType::BREAK) {
    _builder->CreateBr(e);
  } else if (p->get_node_type() == ASTNodeType::CONTINUE) {
    _builder->CreateBr(s);
  } else {
    TAN_ASSERT(false);
  }

  _llvm_value_cache[p] = nullptr;
}

DEFINE_AST_VISITOR_IMPL(CodeGenerator, VarRef) { _llvm_value_cache[p] = cached_visit(p->get_referred()); }

void CodeGenerator::error(ErrorType type, ASTBase *p, const str &message) {
  Error(type, _sm->get_token(p->start()), _sm->get_token(p->end()), message).raise();
}

} // namespace tanlang
