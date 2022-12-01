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
#include "llvm_api/llvm_include.h"

namespace tanlang {

AllocaInst *CodeGenerator::create_block_alloca(BasicBlock *block, llvm::Type *type, size_t size, const str &name) {
  block = &block->getParent()->getEntryBlock();
  IRBuilder<> tmp_builder(block, block->begin());
  if (size <= 1) {
    return tmp_builder.CreateAlloca(type, nullptr, name);
  } else {
    return tmp_builder.CreateAlloca(type, tmp_builder.getInt32((unsigned)size), name);
  }
}

CodeGenerator::CodeGenerator(CompilerSession *cs, TargetMachine *target_machine)
    : _cs(cs), _sm(cs->get_source_manager()), _target_machine(target_machine) {
  _context = new LLVMContext();
  _builder = new IRBuilder<>(*_context);
  _module = new Module(cs->get_filename(), *_context);
  _module->setDataLayout(_target_machine->createDataLayout());
  _module->setTargetTriple(_target_machine->getTargetTriple().str());
  auto opt_level = (llvm::CodeGenOpt::Level)Compiler::compile_config.opt_level;
  _target_machine->setOptLevel(opt_level);

  /// add the current debug info version into the module
  _module->addModuleFlag(Module::Warning, "Dwarf Version", llvm::dwarf::DWARF_VERSION);
  _module->addModuleFlag(Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);

  /// debug related
  _di_builder = new DIBuilder(*_module);
  _di_file = _di_builder->createFile(cs->get_filename(), ".");
  _di_cu = _di_builder->createCompileUnit(llvm::dwarf::DW_LANG_C, _di_file, "tan compiler", false, "", 0);
  _di_scope = {_di_file};

  /// Init noop intrinsic
  /// fn llvm.donothing() : void;
  /// TODO: generate only if used
  Function *func = _module->getFunction("llvm.donothing");
  if (!func) {
    llvm::Type *ret_type = _builder->getVoidTy();
    auto *FT = llvm::FunctionType::get(ret_type, {}, false);
    Function::Create(FT, Function::ExternalWeakLinkage, "llvm.donothing", _module);
  }
}

CodeGenerator::~CodeGenerator() {
  delete _di_builder;
  delete _module;
  delete _builder;
  delete _context;
}

void CodeGenerator::emit_to_file(const str &filename) {
  _di_builder->finalize(); /// important: do this before any pass

  auto opt_level = _target_machine->getOptLevel();
  bool debug = opt_level == llvm::CodeGenOpt::Level::None;

  /// pass manager builder
  auto *pm_builder = new PassManagerBuilder();
  pm_builder->OptLevel = opt_level;
  pm_builder->SizeLevel = 0; // TODO: optimize for size?
  pm_builder->DisableUnrollLoops = debug;
  pm_builder->SLPVectorize = !debug;
  pm_builder->LoopVectorize = !debug;
  pm_builder->RerollLoops = !debug;
  pm_builder->NewGVN = !debug;
  pm_builder->DisableGVNLoadPRE = !debug;
  pm_builder->MergeFunctions = !debug;
  pm_builder->VerifyInput = true;
  pm_builder->VerifyOutput = true;
  auto *tlii = new llvm::TargetLibraryInfoImpl(Triple(_module->getTargetTriple()));
  pm_builder->LibraryInfo = tlii;
  pm_builder->Inliner = llvm::createFunctionInliningPass();

  /// module pass
  PassManager mpm;
  mpm.add(createTargetTransformInfoWrapperPass(_target_machine->getTargetIRAnalysis()));
  pm_builder->populateModulePassManager(mpm);
  mpm.run(*_module);

  /// function pass
  FunctionPassManager fpm(_module);
  fpm.add(createTargetTransformInfoWrapperPass(_target_machine->getTargetIRAnalysis()));
  fpm.add(llvm::createVerifierPass());
  pm_builder->populateFunctionPassManager(fpm);
  fpm.doInitialization();
  for (auto &f : *_module) {
    if (f.getName() == "tan_main") { /// mark tan_main as used, prevent LLVM from deleting it
      llvm::appendToUsed(*_module, {(GlobalValue *)&f});
    }
    fpm.run(f);
  }
  fpm.doFinalization();

  /// generate object files
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

Value *CodeGenerator::codegen(ASTBase *p) {
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
  case ASTNodeType::STRUCT_DECL:
    /// don't do anything
    break;
  default:
    error(p, fmt::format("[DEV] Codegen for {} is not implemented", ASTBase::ASTTypeNames[p->get_node_type()]));
    TAN_ASSERT(false);
    break;
  }

  return _llvm_value_cache[p] = ret;
}

llvm::Value *CodeGenerator::convert_llvm_type_to(Expr *expr, Type *dest) {
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
  } else if (orig->is_bool() && dest->is_int()) { /// bool to int
    return _builder->CreateZExtOrTrunc(loaded, to_llvm_type(dest));
  } else if (orig->is_bool() && dest->is_float()) { /// bool to float
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
  } else if (orig->is_array() && dest->is_pointer()) { /// array to pointer, don't need to do anything
    return loaded;
  } else if (orig->is_array() && dest->is_string()) { /// array to string, don't need to do anything
    return loaded;
  }

  error(expr, "Cannot perform type conversion");
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

llvm::Metadata *CodeGenerator::to_llvm_metadata(Type *p) {
  TAN_ASSERT(p);
  TAN_ASSERT(!p->is_ref());

  auto it = _llvm_metadata_cache.find(p);
  if (it != _llvm_metadata_cache.end()) {
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
  } else if (p->is_enum()) { /// enums
    // TODO IMPORTANT
  } else if (p->is_struct()) { /// struct
    auto member_types = ((StructType *)p)->get_member_types();
    vector<Metadata *> elements(member_types.size(), nullptr);
    for (size_t i = 1; i < member_types.size(); ++i) {
      elements[i] = to_llvm_metadata(member_types[i]);
    }
    ret = _di_builder->createStructType(get_current_di_scope(), p->get_typename(), _di_file,
                                        0, // TODO IMPORTANT: (unsigned) cs->get_source_manager()->get_line(p->loc()),
                                        0, // TODO IMPORTANT: p->get_size_bits(),
                                        0, // TODO IMPORTANT: p->get_align_bits(),
                                        DINode::DIFlags::FlagZero, nullptr, _di_builder->getOrCreateArray(elements), 0,
                                        nullptr, p->get_typename());
  } else if (p->is_array()) { /// array as pointer
    auto *sub = to_llvm_metadata(((ArrayType *)p)->get_element_type());
    ret = _di_builder->createPointerType((DIType *)sub, _target_machine->getPointerSizeInBits(0),
                                         (unsigned)_target_machine->getPointerSizeInBits(0), llvm::None,
                                         p->get_typename());
  } else if (p->is_pointer()) { /// pointer
    auto *sub = to_llvm_metadata(((PointerType *)p)->get_pointee());
    ret = _di_builder->createPointerType((DIType *)sub, _target_machine->getPointerSizeInBits(0),
                                         (unsigned)_target_machine->getPointerSizeInBits(0), llvm::None,
                                         p->get_typename());
  } else {
    TAN_ASSERT(false);
  }

  _llvm_metadata_cache[p] = ret;
  return ret;
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

void CodeGenerator::error(ASTBase *p, const str &message) {
  Error err(_cs->_filename, _sm->get_token(p->loc()), message);
  err.raise();
}

DIScope *CodeGenerator::get_current_di_scope() const { return _di_scope.back(); }
void CodeGenerator::push_di_scope(DIScope *scope) { _di_scope.push_back(scope); }
void CodeGenerator::pop_di_scope() { _di_scope.pop_back(); }

DebugLoc CodeGenerator::debug_loc_of_node(ASTBase *p, MDNode *scope) {
  return DILocation::get(*_context, _sm->get_line(p->loc()), _sm->get_col(p->loc()), scope);
}

Value *CodeGenerator::codegen_func_call(ASTBase *_p) {
  auto *p = ast_cast<FunctionCall>(_p);

  FunctionDecl *callee = p->_callee;
  auto *callee_type = (tanlang::FunctionType *)callee->get_type();
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
    auto expected_ty = callee_type->get_arg_types()[i];
    a = convert_llvm_type_to(actual_arg, expected_ty);
    arg_vals.push_back(a);
  }
  return _builder->CreateCall((llvm::FunctionType *)to_llvm_type(callee->get_type()), _llvm_value_cache[callee],
                              arg_vals);
}

Value *CodeGenerator::codegen_func_prototype(FunctionDecl *p, bool import) {
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
  Function *func = Function::Create((llvm::FunctionType *)to_llvm_type(p->get_type()), linkage, p->get_name(), _module);
  func->setCallingConv(llvm::CallingConv::C);
  return func;
}

Value *CodeGenerator::codegen_func_decl(FunctionDecl *p) {
  auto *func_type = (tanlang::FunctionType *)p->get_type();

  auto ret_ty = func_type->get_return_type();
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
    auto ty = func_type->get_arg_types()[i];
    arg_metas.push_back(to_llvm_metadata(ty));
  }

  /// function implementation
  if (!p->is_external()) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*_context, "func_entry", F);
    _builder->SetInsertPoint(main_block);

    /// debug information
    DIScope *di_scope = get_current_di_scope();
    auto *di_func_t = create_function_debug_info_type(ret_meta, arg_metas);
    DISubprogram *subprogram = _di_builder->createFunction(
        di_scope, func_name, func_name, _di_file, _sm->get_line(p->loc()), di_func_t, _sm->get_col(p->loc()),
        DINode::FlagPrototyped, DISubprogram::SPFlagDefinition, nullptr, nullptr, nullptr);
    F->setSubprogram(subprogram);
    push_di_scope(subprogram);

    /// add all function arguments to scope
    size_t i = 0;
    for (auto &a : F->args()) {
      auto arg_name = p->get_arg_name(i);
      auto *arg_val = codegen(p->get_arg_decls()[i]);
      _builder->CreateStore(&a, arg_val);

      /// create a debug descriptor for the arguments
      auto *arg_meta = to_llvm_metadata(func_type->get_arg_types()[i]);
      llvm::DILocalVariable *di_arg = _di_builder->createParameterVariable(
          subprogram, arg_name, (unsigned)i + 1, _di_file, _sm->get_line(p->loc()), (DIType *)arg_meta, true);
      _di_builder->insertDeclare(arg_val, di_arg, _di_builder->createExpression(),
                                 debug_loc_of_node(p->get_arg_decls()[i], subprogram), _builder->GetInsertBlock());
      ++i;
    }

    /// generate function body
    codegen(p->get_body());

    /// create a return instruction if there is none, the return value is the default value of the return type
    auto *trailing_block = _builder->GetInsertBlock();
    if (trailing_block->sizeWithoutDebug() == 0 || !trailing_block->back().isTerminator()) {
      if (ret_ty->is_void()) {
        _builder->CreateRetVoid();
      } else {
        auto *ret_val = codegen_type_instantiation(ret_ty);
        TAN_ASSERT(ret_val);
        _builder->CreateRet(ret_val);
      }
    }
    pop_di_scope();
  }

  return F;
}

void CodeGenerator::set_current_debug_location(ASTBase *p) {
  unsigned line = _sm->get_line(p->loc()) + 1;
  unsigned col = _sm->get_col(p->loc()) + 1;
  _builder->SetCurrentDebugLocation(DILocation::get(*_context, line, col, this->get_current_di_scope()));
}

Value *CodeGenerator::codegen_bnot(ASTBase *_p) {
  auto p = ast_cast<UnaryOperator>(_p);

  auto *rhs = codegen(p->get_rhs());
  if (!rhs) {
    error(p, "Invalid operand");
  }
  if (p->get_rhs()->is_lvalue()) {
    rhs = _builder->CreateLoad(to_llvm_type(p->get_rhs()->get_type()), rhs);
  }
  return _builder->CreateNot(rhs);
}

Value *CodeGenerator::codegen_lnot(ASTBase *_p) {
  auto p = ast_cast<UnaryOperator>(_p);

  auto *rhs = codegen(p->get_rhs());

  if (!rhs) {
    error(p, "Invalid operand");
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

  error(p, "Invalid operand");
}

Value *CodeGenerator::codegen_return(ASTBase *_p) {
  auto p = ast_cast<Return>(_p);

  auto rhs = p->get_rhs();
  if (rhs) { /// return with value
    Value *result = codegen(rhs);
    if (rhs->is_lvalue()) {
      result = _builder->CreateLoad(to_llvm_type(rhs->get_type()), result);
    }
    _builder->CreateRet(result);
  } else { /// return void
    _builder->CreateRetVoid();
  }
  return nullptr;
}

Value *CodeGenerator::codegen_var_arg_decl(ASTBase *_p) {
  auto p = ast_cast<Decl>(_p);

  llvm::Type *type = to_llvm_type(p->get_type());
  auto *ret = create_block_alloca(_builder->GetInsertBlock(), type, 1, p->get_name());

  /// default value of only var declaration
  if (p->get_node_type() == ASTNodeType::VAR_DECL) {
    auto *default_value = codegen_type_instantiation(p->get_type());
    if (!default_value) {
      error(p, "Fail to instantiate this type");
    }
    _builder->CreateStore(default_value, ret);
  }

  /// debug info
  {
    auto *curr_di_scope = get_current_di_scope();
    auto *arg_meta = to_llvm_metadata(p->get_type());
    auto *di_arg = _di_builder->createAutoVariable(curr_di_scope, p->get_name(), _di_file, _sm->get_line(p->loc()),
                                                   (DIType *)arg_meta);
    _di_builder->insertDeclare(
        ret, di_arg, _di_builder->createExpression(),
        DILocation::get(*_context, _sm->get_line(p->loc()), _sm->get_col(p->loc()), curr_di_scope),
        _builder->GetInsertBlock());
  }
  return ret;
}

Value *CodeGenerator::codegen_address_of(ASTBase *_p) {
  auto p = ast_cast<UnaryOperator>(_p);

  if (!p->get_rhs()->is_lvalue()) {
    error(p, "Cannot get address of rvalue");
  }

  return codegen(p->get_rhs());
}

Value *CodeGenerator::codegen_parenthesis(ASTBase *_p) {
  auto p = ast_cast<Parenthesis>(_p);
  return codegen(p->get_sub());
}

Value *CodeGenerator::codegen_import(ASTBase *_p) {
  auto p = ast_cast<Import>(_p);

  for (FunctionDecl *f : p->get_imported_funcs()) {
    /// do nothing for already defined intrinsics
    auto *func = _module->getFunction(f->get_name());
    if (func) {
      _llvm_value_cache[f] = func;
    }
    _llvm_value_cache[f] = codegen_func_prototype(f);
  }

  return nullptr;
}

Value *CodeGenerator::codegen_intrinsic(Intrinsic *p) {
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

Value *CodeGenerator::codegen_constructor(Constructor *p) {
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

Value *CodeGenerator::codegen_type_instantiation(Type *p) {
  // TODO: TAN_ASSERT(p->get_constructor());
  TAN_ASSERT(!p->is_ref());

  Value *ret = nullptr;
  if (p->is_primitive()) { /// primitive types
    int size_bits = p->get_size_bits();

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
    ret =
        codegen_constructor(BasicConstructor::CreateArrayConstructor(SrcLoc(0), ((ArrayType *)p)->get_element_type()));
  } else if (p->is_pointer()) { /// the pointer literal is nullptr
    ret = codegen_constructor(
        BasicConstructor::CreateNullPointerConstructor(SrcLoc(0), ((PointerType *)p)->get_pointee()));
  } else {
    TAN_ASSERT(false);
  }

  return ret;
}

Value *CodeGenerator::codegen_literals(ASTBase *_p) {
  auto p = ast_cast<Literal>(_p);

  llvm::Type *type = to_llvm_type(p->get_type());
  Value *ret = nullptr;
  Type *ptype = p->get_type();
  if (ptype->is_primitive()) { /// primitive types
    int size_bits = ptype->get_size_bits();

    if (ptype->is_char()) { // NOTE: must be before is_int() check because char is technically an integer
      ret = ConstantInt::get(type, ast_cast<CharLiteral>(p)->get_value());
    } else if (ptype->is_int()) {
      auto pp = ast_cast<IntegerLiteral>(p);
      ret = ConstantInt::get(_builder->getIntNTy((unsigned)size_bits), pp->get_value(), !pp->is_unsigned());
    } else if (ptype->is_bool()) {
      auto pp = ast_cast<BoolLiteral>(p);
      ret = ConstantInt::get(type, (uint64_t)pp->get_value());
    } else if (ptype->is_float()) {
      ret = ConstantFP::get(type, ast_cast<FloatLiteral>(p)->get_value());
    } else {
      TAN_ASSERT(false);
    }
  } else if (ptype->is_string()) { /// str as char*
    ret = _builder->CreateGlobalStringPtr(ast_cast<StringLiteral>(p)->get_value());
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
    ret = create_block_alloca(_builder->GetInsertBlock(), e_type, n, "const_array");
    for (size_t i = 0; i < n; ++i) {
      auto *idx = _builder->getInt32((unsigned)i);
      auto *e_val = codegen(elements[i]);
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

Value *CodeGenerator::codegen_stmt(ASTBase *_p) {
  auto p = ast_cast<CompoundStmt>(_p);

  for (const auto &e : p->get_children()) {
    codegen(e);
  }
  return nullptr;
}

Value *CodeGenerator::codegen_uop(ASTBase *_p) {
  auto p = ast_cast<UnaryOperator>(_p);
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
    ret = codegen(rhs);
    break;
  case UnaryOpKind::MINUS: {
    auto *r = codegen(rhs);
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
  return ret;
}

Value *CodeGenerator::codegen_bop(ASTBase *_p) {
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

Value *CodeGenerator::codegen_assignment(ASTBase *_p) {
  auto p = ast_cast<Assignment>(_p);

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

  _builder->CreateStore(from, to);
  return to;
}

Value *CodeGenerator::codegen_arithmetic(ASTBase *_p) {
  auto p = ast_cast<BinaryOperator>(_p);

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

Value *CodeGenerator::codegen_comparison(ASTBase *_p) {
  auto p = ast_cast<BinaryOperator>(_p);

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

Value *CodeGenerator::codegen_relop(ASTBase *_p) {
  auto p = ast_cast<BinaryOperator>(_p);

  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();
  Value *l = codegen(lhs);
  Value *r = codegen(rhs);

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

Value *CodeGenerator::codegen_cast(ASTBase *_p) {
  auto p = ast_cast<Cast>(_p);
  auto lhs = p->get_lhs();
  auto *dest_type = to_llvm_type(p->get_type());

  Value *val = codegen(lhs);
  TAN_ASSERT(val);

  Value *ret = nullptr;
  val = convert_llvm_type_to(lhs, p->get_type()); // lvalue will be loaded here
  if (lhs->is_lvalue()) {
    ret = create_block_alloca(_builder->GetInsertBlock(), dest_type, 1, "casted");
    _builder->CreateStore(val, ret);
  } else {
    ret = val;
  }
  return ret;
}

Value *CodeGenerator::codegen_var_ref(ASTBase *_p) {
  auto p = ast_cast<VarRef>(_p);
  return codegen(p->get_referred());
}

Value *CodeGenerator::codegen_identifier(ASTBase *_p) {
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

Value *CodeGenerator::codegen_binary_or_unary(ASTBase *_p) {
  auto p = ast_cast<BinaryOrUnary>(_p);
  return codegen(p->get_expr_ptr());
}

Value *CodeGenerator::codegen_break_continue(ASTBase *_p) {
  auto *p = ast_cast<BreakContinue>(_p);
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
  return nullptr;
}

Value *CodeGenerator::codegen_loop(ASTBase *_p) {
  auto p = ast_cast<Loop>(_p);
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
    p->_loop_start = BasicBlock::Create(*_context, "loop", func);
    BasicBlock *loop_body = BasicBlock::Create(*_context, "loop_body", func);
    p->_loop_end = BasicBlock::Create(*_context, "after_loop", func);

    /// start loop
    // create a br instruction if there is no terminator instruction at the end of this block
    if (!_builder->GetInsertBlock()->back().isTerminator()) {
      _builder->CreateBr(p->_loop_start);
    }

    /// condition
    _builder->SetInsertPoint(p->_loop_start);
    auto *cond = codegen(p->get_predicate());
    if (!cond) {
      error(p, "Expected a condition expression");
    }
    _builder->CreateCondBr(cond, loop_body, p->_loop_end);

    /// loop body
    _builder->SetInsertPoint(loop_body);
    codegen(p->get_body());

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

  return nullptr;
}

Value *CodeGenerator::codegen_if(ASTBase *_p) {
  auto p = ast_cast<If>(_p);

  Function *func = _builder->GetInsertBlock()->getParent();
  BasicBlock *merge_bb = BasicBlock::Create(*_context, "endif");
  size_t n = p->get_num_branches();

  /// create basic blocks
  vector<BasicBlock *> cond_blocks(n);
  vector<BasicBlock *> then_blocks(n);
  for (size_t i = 0; i < n; ++i) {
    cond_blocks[i] = BasicBlock::Create(*_context, "cond", func);
    then_blocks[i] = BasicBlock::Create(*_context, "branch", func);
  }

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
      Value *cond_v = codegen(cond);
      cond_v = load_if_is_lvalue(cond);
      if (i < n - 1) {
        _builder->CreateCondBr(cond_v, then_blocks[i], cond_blocks[i + 1]);
      } else {
        _builder->CreateCondBr(cond_v, then_blocks[i], merge_bb);
      }
    }

    /// then clause
    _builder->SetInsertPoint(then_blocks[i]);
    codegen(p->get_branch(i));

    /// go to merge block if there is no terminator instruction at the end of then
    if (!_builder->GetInsertBlock()->back().isTerminator()) {
      _builder->CreateBr(merge_bb);
    }
  }

  /// emit merge block
  func->getBasicBlockList().push_back(merge_bb);
  _builder->SetInsertPoint(merge_bb);
  return nullptr;
}

Value *CodeGenerator::codegen_member_access(MemberAccess *p) {
  auto lhs = p->get_lhs();
  auto rhs = p->get_rhs();

  auto *lhs_val = codegen(lhs);

  Value *ret = nullptr;
  switch (p->_access_type) {
  case MemberAccess::MemberAccessBracket: {
    lhs_val = load_if_is_lvalue(lhs);

    /// currently only support array bracket access
    auto *lhs_type = (tanlang::ArrayType *)lhs->get_type();
    codegen(rhs);
    auto *rhs_val = load_if_is_lvalue(rhs);

    ret = _builder->CreateGEP(to_llvm_type(lhs_type->get_element_type()), lhs_val, rhs_val, "bracket_access");
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
    unsigned idx = (unsigned)p->_access_idx;
    ret = _builder->CreateStructGEP(to_llvm_type(st), lhs_val, idx, "member_variable");
    break;
  }
  case MemberAccess::MemberAccessMemberFunction:
    // TODO: codegen for member function call
    ret = codegen(rhs);
    break;
  case MemberAccess::MemberAccessEnumValue:
    // TODO: codegen for enum member access
    break;
  default:
    TAN_ASSERT(false);
  }

  return ret;
}

Value *CodeGenerator::codegen_ptr_deref(UnaryOperator *p) {
  auto *rhs = p->get_rhs();
  Value *val = codegen(rhs);
  TAN_ASSERT(val->getType()->isPointerTy());

  /// load only if the pointer itself is an lvalue, so that the value after deref is always an lvalue
  if (rhs->is_lvalue()) {
    val = _builder->CreateLoad(to_llvm_type(rhs->get_type()), val, "ptr_deref");
  }
  return val;
}

} // namespace tanlang
