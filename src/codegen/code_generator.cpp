#include "src/codegen/code_generator.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_context.h"
#include "src/ast/constructor.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/ast/intrinsic.h"
#include "src/analysis/type_system.h"
#include "src/common.h"
#include "src/llvm_include.h"
#include "compiler_session.h"

namespace tanlang {

// TODO: extract into static functions that don't depend on ASTBase classes so that code can be reused (such as
//   in intrinsic.cpp
class CodeGeneratorImpl {
public:
  CodeGeneratorImpl() = delete;

  explicit CodeGeneratorImpl(CompilerSession *cs, ASTContext *ctx)
      : _cs(cs), _ctx(ctx), _sm(cs->get_source_manager()) {}

  Value *codegen(ASTBase *p) {
    if (p->_llvm_value) {
      return p->_llvm_value;
    }

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
        ret = codegen_literals(p);
        break;
      case ASTNodeType::INTRINSIC:
        ret = codegen_intrinsic(ast_must_cast<Intrinsic>(p));
        break;
      case ASTNodeType::FUNC_DECL:
        ret = codegen_func_decl(ast_must_cast<FunctionDecl>(p));
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
      case ASTNodeType::TY:
        ret = codegen_type_instantiation(ast_cast<ASTType>(p));
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
    p->_llvm_value = ret;
    return ret;
  }

  static Constant *CodegenIntegerLiteral(CompilerSession *cs, uint64_t val, size_t bit_size, bool is_unsigned = false) {
    return ConstantInt::get(cs->_builder->getIntNTy((unsigned) bit_size), val, !is_unsigned);
  }

  static Function *CodegenFuncPrototype(CompilerSession *cs,
      ASTType *ret_ty,
      str name,
      vector<ASTType *> arg_tys,
      bool is_external,
      bool is_public,
      bool is_imported) {
    Type *ret_type = TypeSystem::ToLLVMType(cs, ret_ty);

    /// set function arg types
    vector<Type *> args{};
    std::for_each(arg_tys.begin(), arg_tys.end(), [&](ASTType *t) {
      args.push_back(TypeSystem::ToLLVMType(cs, t));
    });

    /// create function prototype
    FunctionType *FT = FunctionType::get(ret_type, args, false);
    auto linkage = Function::InternalLinkage;
    if (is_external) {
      linkage = Function::ExternalWeakLinkage;
    }
    if (is_public) {
      if (is_imported) {
        linkage = Function::ExternalWeakLinkage;
      } else {
        linkage = Function::ExternalLinkage;
      }
    }
    Function *func = Function::Create(FT, linkage, name, cs->get_module());
    func->setCallingConv(llvm::CallingConv::C);

    return func;
  }

private:
  CompilerSession *_cs = nullptr;
  ASTContext *_ctx = nullptr;
  SourceManager *_sm = nullptr;

private:
  Value *codegen_func_call(ASTBase *_p) {
    auto p = ast_must_cast<FunctionCall>(_p);

    FunctionDecl *callee = p->_callee;
    size_t n = callee->get_n_args();

    /// args
    vector<Value *> arg_vals;
    for (size_t i = 0; i < n; ++i) {
      auto actual_arg = p->_args[i];
      auto *a = codegen(actual_arg);
      if (!a) {
        report_error(actual_arg, "Invalid function call argument");
      }

      /// implicit cast
      auto expected_ty = callee->get_arg_type(i);
      a = TypeSystem::ConvertTo(_cs, a, actual_arg->get_type(), expected_ty);
      arg_vals.push_back(a);
    }
    return p->_llvm_value = _cs->_builder->CreateCall(codegen(callee), arg_vals);
  }

  Value *codegen_func_prototype(FunctionDecl *p, bool import = false) {
    /// set function arg types
    vector<ASTType *> arg_types{};
    for (size_t i = 0; i < p->get_n_args(); ++i) {
      arg_types.push_back(p->get_arg_type(i));
    }

    p->_llvm_value =
        CodegenFuncPrototype(_cs, p->get_ret_ty(), p->get_name(), arg_types, p->is_external(), p->is_public(), import);
    return p->_llvm_value;
  }

  Value *codegen_func_decl(FunctionDecl *p) {
    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto ret_ty = p->get_ret_ty();
    Metadata *ret_meta = TypeSystem::ToLLVMMeta(_cs, ret_ty);

    /// get function name
    str func_name = p->get_name();
    /// rename to "tan_main", as it will be called by the real main function in runtime/main.cpp
    if (func_name == "main") {
      p->set_name(func_name = "tan_main");
      p->set_external(true);
    }

    /// generate prototype
    auto *F = (Function *) codegen_func_prototype(p);

    /// set function arg types
    vector<Metadata *> arg_metas;
    for (size_t i = 0; i < p->get_n_args(); ++i) {
      auto ty = p->get_arg_type(i);
      arg_metas.push_back(TypeSystem::ToLLVMMeta(_cs, ty));
    }

    /// function implementation
    if (!p->is_external()) {
      /// create a new basic block to start insertion into
      BasicBlock *main_block = BasicBlock::Create(*_cs->get_context(), "func_entry", F);
      builder->SetInsertPoint(main_block);

      /// debug information
      DIScope *di_scope = _cs->get_current_di_scope();
      auto *di_file = _cs->get_di_file();
      auto *di_func_t = TypeSystem::CreateFunctionDIType(_cs, ret_meta, arg_metas);
      DISubprogram *subprogram = _cs->_di_builder
          ->createFunction(di_scope,
              func_name,
              func_name,
              di_file,
              (unsigned) _sm->get_line(p->get_loc()),
              di_func_t,
              (unsigned) _sm->get_col(p->get_loc()),
              DINode::FlagPrototyped,
              DISubprogram::SPFlagDefinition,
              nullptr,
              nullptr,
              nullptr);
      F->setSubprogram(subprogram);
      _cs->push_di_scope(subprogram);
      /// reset debug emit location
      builder->SetCurrentDebugLocation(DebugLoc());

      /// add all function arguments to scope
      size_t i = 0;
      for (auto &a : F->args()) {
        auto arg_name = p->get_arg_name(i);
        auto *arg_val = codegen(p->get_arg_decls()[i]);
        builder->CreateStore(&a, arg_val);
        /// create a debug descriptor for the arguments
        auto *arg_meta = TypeSystem::ToLLVMMeta(_cs, p->get_arg_type(i));
        llvm::DILocalVariable *di_arg = _cs->_di_builder
            ->createParameterVariable(subprogram,
                arg_name,
                (unsigned) i + 1,
                di_file,
                (unsigned) _sm->get_line(p->get_loc()),
                (DIType *) arg_meta,
                true);
        _cs->_di_builder
            ->insertDeclare(arg_val,
                di_arg,
                _cs->_di_builder->createExpression(),
                llvm::DebugLoc::get((unsigned) _sm->get_line(p->get_loc()),
                    (unsigned) _sm->get_col(p->get_loc()),
                    subprogram),
                builder->GetInsertBlock());
        ++i;
      }

      /// set debug emit location to function body
      builder->SetCurrentDebugLocation(llvm::DebugLoc::get((unsigned) _sm->get_line(p->get_body()->get_loc()),
          (unsigned) _sm->get_col(p->get_body()->get_loc()),
          subprogram));

      /// generate function body
      codegen(p->get_body());

      /// create a return instruction if there is none, the return value is the default value of the return type
      if (!builder->GetInsertBlock()->back().isTerminator()) {
        if (ret_ty->get_ty() == Ty::VOID) {
          builder->CreateRetVoid();
        } else {
          auto *ret_val = codegen_type_instantiation(ret_ty);
          TAN_ASSERT(ret_val);
          builder->CreateRet(ret_val);
        }
      }
      _cs->pop_di_scope();
    }

    return p->_llvm_value = F;
  }

  void set_current_debug_location(ASTBase *p) {
    _cs->set_current_debug_location(_sm->get_line(p->get_loc()), _sm->get_col(p->get_loc()));
  }

  Value *codegen_bnot(ASTBase *_p) {
    auto p = ast_must_cast<UnaryOperator>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto *rhs = codegen(p->get_rhs());
    if (!rhs) {
      report_error(p, "Invalid operand");
    }
    if (p->get_rhs()->get_type()->is_lvalue()) {
      rhs = builder->CreateLoad(rhs);
    }
    return p->_llvm_value = builder->CreateNot(rhs);
  }

  Value *codegen_lnot(ASTBase *_p) {
    auto p = ast_must_cast<UnaryOperator>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto *rhs = codegen(p->get_rhs());

    if (!rhs) {
      report_error(p, "Invalid operand");
    }

    if (p->get_rhs()->get_type()->is_lvalue()) {
      rhs = builder->CreateLoad(rhs);
    }
    /// get value size in bits
    auto size_in_bits = rhs->getType()->getPrimitiveSizeInBits();
    if (rhs->getType()->isFloatingPointTy()) {
      p->_llvm_value = builder->CreateFCmpOEQ(rhs, ConstantFP::get(builder->getFloatTy(), 0.0f));
    } else if (rhs->getType()->isSingleValueType()) {
      p->_llvm_value =
          builder->CreateICmpEQ(rhs, ConstantInt::get(builder->getIntNTy((unsigned) size_in_bits), 0, false));
    } else { report_error(p, "Invalid operand"); }
    return p->_llvm_value;
  }

  Value *codegen_return(ASTBase *_p) {
    auto p = ast_must_cast<Return>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto rhs = p->get_rhs();
    auto *result = codegen(rhs);
    if (rhs->get_type()->is_lvalue()) {
      result = builder->CreateLoad(result, "ret");
    }
    builder->CreateRet(result);
    return nullptr;
  }

  Value *codegen_var_arg_decl(ASTBase *_p) {
    auto p = ast_must_cast<Decl>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    if (!p->get_type()->is_resolved()) {
      report_error(p, "Unknown type");
    }
    Type *type = TypeSystem::ToLLVMType(_cs, p->get_type());
    p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), type, 1, p->get_name());

    /// default value of only var declaration
    if (p->get_node_type() == ASTNodeType::VAR_DECL) {
      auto *default_value = codegen_type_instantiation(p->get_type());
      if (!default_value) {
        report_error(p, "Fail to instantiate this type");
      }
      builder->CreateStore(default_value, p->_llvm_value);
    }

    /// debug info
    {
      auto *di_builder = _cs->_di_builder;
      auto *curr_di_scope = _cs->get_current_di_scope();
      auto *arg_meta = TypeSystem::ToLLVMMeta(_cs, p->get_type());
      auto *di_arg = di_builder->createAutoVariable(curr_di_scope,
          p->get_name(),
          _cs->get_di_file(),
          (unsigned) _sm->get_line(p->get_loc()),
          (DIType *) arg_meta);
      di_builder->insertDeclare(p->_llvm_value,
          di_arg,
          _cs->_di_builder->createExpression(),
          llvm::DebugLoc::get((unsigned) _sm->get_line(p->get_loc()),
              (unsigned) _sm->get_col(p->get_loc()),
              curr_di_scope),
          builder->GetInsertBlock());
    }
    return p->_llvm_value;
  }

  Value *codegen_address_of(ASTBase *_p) {
    auto p = ast_must_cast<UnaryOperator>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto *val = codegen(p->get_rhs());
    if (p->get_rhs()->get_type()->is_lvalue()) { /// lvalue, the val itself is a pointer to real value
      p->_llvm_value = val;
    } else { /// rvalue, create an anonymous variable, and get address of it
      p->_llvm_value = create_block_alloca(builder->GetInsertBlock(), val->getType(), 1, "anonymous");
      builder->CreateStore(val, p->_llvm_value);
    }
    return p->_llvm_value;
  }

  Value *codegen_parenthesis(ASTBase *_p) {
    auto p = ast_must_cast<Parenthesis>(_p);

    set_current_debug_location(p);

    return p->_llvm_value = codegen(p->get_sub());
  }

  Value *codegen_import(ASTBase *_p) {
    auto p = ast_must_cast<Import>(_p);

    set_current_debug_location(p);
    for (FunctionDecl *f: p->get_imported_funcs()) {
      /// do nothing for already defined intrinsics
      auto *func = _cs->get_module()->getFunction(f->get_name());
      if (!func) {
        codegen_func_prototype(f);
      } else {
        f->_llvm_value = func;
      }
    }
    return nullptr;
  }

  Value *codegen_intrinsic(Intrinsic *p) {
    set_current_debug_location(p);

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
        ret = codegen(cast_ptr<BasicConstructor>(p)->get_value());
        break;
      case ConstructorType::STRUCT: {
        auto *ctr = cast_ptr<StructConstructor>(p);
        vector<Constructor *> sub_ctrs = ctr->get_member_constructors();
        vector<Constant *> values{};
        values.reserve(sub_ctrs.size());
        for (auto *c: sub_ctrs) {
          values.push_back((Constant *) codegen_constructor(c));
        }
        ret = ConstantStruct::get((StructType *) TypeSystem::ToLLVMType(_cs, ctr->get_struct_type()), values);
        break;
      }
      default:
        TAN_ASSERT(false);
        break;
    }
    return ret;
  }

  Value *codegen_type_instantiation(ASTType *p) {
    TAN_ASSERT(p->is_resolved());
    // TODO: TAN_ASSERT(p->get_constructor());

    Ty base = TY_GET_BASE(p->get_ty());
    Value *ret = nullptr;
    Type *type = TypeSystem::ToLLVMType(_cs, p);
    switch (base) {
      case Ty::ENUM:
      case Ty::BOOL:
      case Ty::INT:
      case Ty::CHAR:
      case Ty::DOUBLE:
      case Ty::FLOAT:
      case Ty::STRING:
      case Ty::ARRAY:
        ret = codegen_constructor(p->get_constructor());
        break;
      case Ty::VOID:
        TAN_ASSERT(false);
      case Ty::STRUCT: {
        // TODO: use codegen_constructor()
        vector<Constant *> values{};
        size_t n = p->get_sub_types().size();
        for (size_t i = 0; i < n; ++i) {
          values.push_back((llvm::Constant *) codegen(p->get_sub_types()[i]));
        }
        ret = ConstantStruct::get((StructType *) TypeSystem::ToLLVMType(_cs, p), values);
        break;
      }
      case Ty::POINTER:
        // TODO: use codegen_constructor()
        ret = ConstantPointerNull::get((PointerType *) type);
        break;
      case Ty::TYPE_REF: {
        ret = codegen_type_instantiation(p->get_canonical_type());
        break;
      }
      default:
        TAN_ASSERT(false);
    }
    return ret;
  }

  Value *codegen_literals(ASTBase *_p) {
    auto p = ast_must_cast<Literal>(_p);

    set_current_debug_location(p);
    auto *builder = _cs->_builder;

    Type *type = TypeSystem::ToLLVMType(_cs, p->get_type());
    Value *ret = nullptr;
    Ty t = TY_GET_BASE(p->get_type()->get_ty());
    switch (t) {
      case Ty::CHAR:
        ret = ConstantInt::get(type, ast_must_cast<CharLiteral>(p)->get_value());
        break;
      case Ty::BOOL: {
        auto pp = ast_must_cast<BoolLiteral>(p);
        ret = ConstantInt::get(type, (uint64_t) pp->get_value());
        break;
      }
      case Ty::ENUM:
      case Ty::INT: {
        auto pp = ast_must_cast<IntegerLiteral>(p);
        ret = CodegenIntegerLiteral(_cs, pp->get_value(), pp->get_type()->get_size_bits(), pp->is_unsigned());
        break;
      }
      case Ty::STRING:
        ret = builder->CreateGlobalStringPtr(ast_must_cast<StringLiteral>(p)->get_value());
        break;
      case Ty::FLOAT:
      case Ty::DOUBLE:
        ret = ConstantFP::get(type, ast_must_cast<FloatLiteral>(p)->get_value());
        break;
      case Ty::ARRAY: {
        auto arr = ast_must_cast<ArrayLiteral>(p);

        /// element type
        auto elements = arr->get_elements();
        vector<ASTType *> sub_types = arr->get_type()->get_sub_types();
        TAN_ASSERT(!sub_types.empty());
        auto *e_type = TypeSystem::ToLLVMType(_cs, sub_types[0]);

        /// codegen element values
        size_t n = elements.size();
        ret = create_block_alloca(builder->GetInsertBlock(), e_type, n, "const_array");
        for (size_t i = 0; i < n; ++i) {
          auto *idx = builder->getInt32((unsigned) i);
          auto *e_val = codegen(elements[i]);
          auto *e_ptr = builder->CreateGEP(ret, idx);
          builder->CreateStore(e_val, e_ptr);
        }
        break;
      }
      default:
        TAN_ASSERT(false);
    }
    return ret;
  }

  // FIXME: write an ASTBaseError class
  Value *codegen_stmt(ASTBase *_p) {
    auto p = ast_must_cast<CompoundStmt>(_p);

    for (const auto &e : p->get_children()) {
      codegen(e);
    }
    return nullptr;
  }

  Value *codegen_uop(ASTBase *_p) {
    auto p = ast_must_cast<UnaryOperator>(_p);
    Value *ret = nullptr;

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

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
        if (rhs->get_type()->is_lvalue()) {
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
    auto p = ast_must_cast<BinaryOperator>(_p);
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
        // TODO: implement codegen of the above operators
        TAN_ASSERT(false);
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
        ret = codegen_member_access(ast_must_cast<MemberAccess>(p));
        break;
      default:
        TAN_ASSERT(false);
        break;
    }

    return p->_llvm_value = ret;
  }

  Value *codegen_assignment(ASTBase *_p) {
    auto p = ast_must_cast<Assignment>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    /// _codegen the lhs and rhs
    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();
    Value *from = codegen(rhs);
    Value *to = codegen(lhs);
    if (!from) { report_error(lhs, "Invalid expression for right-hand operand of the assignment"); }
    if (!to) { report_error(rhs, "Invalid left-hand operand of the assignment"); }

    // type of lhs is the same as type of the assignment
    if (!p->get_type()->is_lvalue()) { report_error(lhs, "Value can only be assigned to lvalue"); }

    from = TypeSystem::ConvertTo(_cs, from, rhs->get_type(), p->get_type());
    builder->CreateStore(from, to);
    p->_llvm_value = to;
    return to;
  }

  Value *codegen_arithmetic(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOperator>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    /// binary operator
    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();
    Value *l = codegen(lhs);
    Value *r = codegen(rhs);
    if (!l) { report_error(lhs, "Invalid expression for right-hand operand"); }
    if (!r) { report_error(rhs, "Invalid expression for left-hand operand"); }

    if (p->_dominant_idx == 0) {
      r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), lhs->get_type());
      l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), lhs->get_type());
    } else {
      l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), rhs->get_type());
      r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), rhs->get_type());
    }

    if (l->getType()->isFloatingPointTy()) {
      /// float arithmetic
      switch (p->get_op()) {
        case BinaryOpKind::MULTIPLY:
          p->_llvm_value = builder->CreateFMul(l, r, "mul_tmp");
          break;
        case BinaryOpKind::DIVIDE:
          p->_llvm_value = builder->CreateFDiv(l, r, "div_tmp");
          break;
        case BinaryOpKind::SUM:
          p->_llvm_value = builder->CreateFAdd(l, r, "sum_tmp");
          break;
        case BinaryOpKind::SUBTRACT:
          p->_llvm_value = builder->CreateFSub(l, r, "sub_tmp");
          break;
        case BinaryOpKind::MOD:
          p->_llvm_value = builder->CreateFRem(l, r, "mod_tmp");
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
    } else {
      /// integer arithmetic
      switch (p->get_op()) {
        case BinaryOpKind::MULTIPLY:
          p->_llvm_value = builder->CreateMul(l, r, "mul_tmp");
          break;
        case BinaryOpKind::DIVIDE: {
          auto ty = lhs->get_type();
          if (ty->is_unsigned()) {
            p->_llvm_value = builder->CreateUDiv(l, r, "div_tmp");
          } else {
            p->_llvm_value = builder->CreateSDiv(l, r, "div_tmp");
          }
          break;
        }
        case BinaryOpKind::SUM:
          p->_llvm_value = builder->CreateAdd(l, r, "sum_tmp");
          break;
        case BinaryOpKind::SUBTRACT:
          p->_llvm_value = builder->CreateSub(l, r, "sub_tmp");
          break;
        case BinaryOpKind::MOD: {
          auto ty = lhs->get_type();
          if (ty->is_unsigned()) {
            p->_llvm_value = builder->CreateURem(l, r, "mod_tmp");
          } else {
            p->_llvm_value = builder->CreateSRem(l, r, "mod_tmp");
          }
          break;
        }
        default:
          TAN_ASSERT(false);
          break;
      }
    }
    return p->_llvm_value;
  }

  Value *codegen_comparison(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOperator>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();
    Value *l = codegen(lhs);
    Value *r = codegen(rhs);
    if (!l) { report_error(lhs, "Invalid expression for right-hand operand"); }
    if (!r) { report_error(rhs, "Invalid expression for left-hand operand"); }

    bool is_signed = true;
    if (p->_dominant_idx == 0) {
      r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), lhs->get_type());
      l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), lhs->get_type());
      is_signed = !lhs->get_type()->is_unsigned();
    } else {
      l = TypeSystem::ConvertTo(_cs, l, lhs->get_type(), rhs->get_type());
      r = TypeSystem::ConvertTo(_cs, r, rhs->get_type(), rhs->get_type());
      is_signed = !rhs->get_type()->is_unsigned();
    }

    if (l->getType()->isFloatingPointTy()) {
      switch (p->get_op()) {
        case BinaryOpKind::EQ:
          p->_llvm_value = builder->CreateFCmpOEQ(l, r, "eq");
          break;
        case BinaryOpKind::NE:
          p->_llvm_value = builder->CreateFCmpONE(l, r, "ne");
          break;
        case BinaryOpKind::GT:
          p->_llvm_value = builder->CreateFCmpOGT(l, r, "gt");
          break;
        case BinaryOpKind::GE:
          p->_llvm_value = builder->CreateFCmpOGE(l, r, "ge");
          break;
        case BinaryOpKind::LT:
          p->_llvm_value = builder->CreateFCmpOLT(l, r, "lt");
          break;
        case BinaryOpKind::LE:
          p->_llvm_value = builder->CreateFCmpOLE(l, r, "le");
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
    } else {
      switch (p->get_op()) {
        case BinaryOpKind::EQ:
          p->_llvm_value = builder->CreateICmpEQ(l, r, "eq");
          break;
        case BinaryOpKind::NE:
          p->_llvm_value = builder->CreateICmpNE(l, r, "ne");
          break;
        case BinaryOpKind::GT:
          if (is_signed) {
            p->_llvm_value = builder->CreateICmpSGT(l, r, "gt");
          } else {
            p->_llvm_value = builder->CreateICmpUGT(l, r, "gt");
          }
          break;
        case BinaryOpKind::GE:
          if (is_signed) {
            p->_llvm_value = builder->CreateICmpSGE(l, r, "ge");
          } else {
            p->_llvm_value = builder->CreateICmpUGE(l, r, "ge");
          }
          break;
        case BinaryOpKind::LT:
          if (is_signed) {
            p->_llvm_value = builder->CreateICmpSLT(l, r, "lt");
          } else {
            p->_llvm_value = builder->CreateICmpULT(l, r, "lt");
          }
          break;
        case BinaryOpKind::LE:
          if (is_signed) {
            p->_llvm_value = builder->CreateICmpSLE(l, r, "le");
          } else {
            p->_llvm_value = builder->CreateICmpULE(l, r, "le");
          }
          break;
        default:
          TAN_ASSERT(false);
          break;
      }
    }
    return p->_llvm_value;
  }

  Value *codegen_cast(ASTBase *_p) {
    auto p = ast_must_cast<Cast>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto lhs = p->get_lhs();
    auto *dest_type = TypeSystem::ToLLVMType(_cs, p->get_type());

    Value *val = codegen(lhs);
    if (!val) { report_error(lhs, "Invalid expression for left-hand operand"); }

    Value *ret = nullptr;

    // lvalue will be loaded here
    val = TypeSystem::ConvertTo(_cs, val, p->get_lhs()->get_type(), p->get_type());
    if (lhs->get_type()->is_lvalue()) {
      ret = create_block_alloca(builder->GetInsertBlock(), dest_type, 1, "casted");
      builder->CreateStore(val, ret);
    } else {
      ret = val;
    }

    return p->_llvm_value = ret;
  }

  Value *codegen_var_ref(ASTBase *_p) {
    auto p = ast_must_cast<VarRef>(_p);
    set_current_debug_location(p);
    return codegen(p->get_referred());
  }

  Value *codegen_identifier(ASTBase *_p) {
    auto p = ast_must_cast<Identifier>(_p);
    set_current_debug_location(p);

    switch (p->get_id_type()) {
      case IdentifierType::ID_VAR_DECL:
        p->_llvm_value = codegen(p->get_var_ref());
        break;
      case IdentifierType::ID_TYPE_DECL:
        p->_llvm_value = nullptr;
        break;
      default:
        TAN_ASSERT(false);
        break;
    }
    return p->_llvm_value;
  }

  Value *codegen_binary_or_unary(ASTBase *_p) {
    auto p = ast_must_cast<BinaryOrUnary>(_p);
    set_current_debug_location(p);
    return p->_llvm_value = codegen(p->get_generic_ptr());
  }

  Value *codegen_break_continue(ASTBase *_p) {
    auto *p = ast_must_cast<BreakContinue>(_p);
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
    auto p = ast_must_cast<Loop>(_p);
    auto *builder = _cs->_builder;

    set_current_debug_location(p);
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
        report_error(p, "Expected a condition expression");
      }
      // TODO: do this in analysis phase, and remove ASTContext from this
      cond = TypeSystem::ConvertTo(_cs,
          cond,
          p->get_predicate()->get_type(),
          ASTType::CreateAndResolve(_ctx, p->get_loc(), Ty::BOOL));
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
    auto p = ast_must_cast<If>(_p);

    auto *builder = _cs->_builder;
    set_current_debug_location(p);

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
      if (!cond) { /// else clause, immediately go to then block
        TAN_ASSERT(i == n - 1); /// only the last branch can be an else
        builder->CreateBr(then_blocks[i]);
      } else {
        Value *cond_v = codegen(cond);
        if (!cond_v) { report_error(p, "Invalid condition expression "); }
        /// convert to bool
        // TODO: do this in analysis phase, and remove ASTContext from this
        cond_v = TypeSystem::ConvertTo(_cs,
            cond_v,
            cond->get_type(),
            ASTType::CreateAndResolve(_ctx, cond->get_loc(), Ty::BOOL));
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
    set_current_debug_location(p);

    auto lhs = p->get_lhs();
    auto rhs = p->get_rhs();

    auto *from = codegen(lhs);
    Value *ret;
    switch (p->_access_type) {
      case MemberAccess::MemberAccessBracket: {
        if (lhs->get_type()->is_lvalue()) { from = builder->CreateLoad(from); }
        auto *rhs_val = codegen(rhs);
        if (rhs->get_type()->is_lvalue()) { rhs_val = builder->CreateLoad(rhs_val); }
        ret = builder->CreateGEP(from, rhs_val, "bracket_access");
        break;
      }
      case MemberAccess::MemberAccessMemberVariable: {
        if (lhs->get_type()->is_lvalue() && lhs->get_type()->is_ptr() && lhs->get_type()->get_contained_ty()) {
          /// auto dereference pointers
          from = builder->CreateLoad(from);
        }
        ret = builder->CreateStructGEP(from, (unsigned) p->_access_idx, "member_variable");
        break;
      }
      case MemberAccess::MemberAccessMemberFunction:
        ret = codegen(rhs);
        break;
      case MemberAccess::MemberAccessEnumValue: {
        str enum_name = ast_must_cast<Identifier>(lhs)->get_name();
        auto *enum_decl = ast_must_cast<EnumDecl>(_ctx->get_type_decl(enum_name));
        str element_name = ast_must_cast<Identifier>(rhs)->get_name();
        int64_t val = enum_decl->get_value(element_name);

        ret = CodegenIntegerLiteral(_cs, (uint64_t) val, enum_decl->get_type()->get_size_bits());
        break;
      }
      default:
        TAN_ASSERT(false);
    }

    return p->_llvm_value = ret;
  }

  Value *codegen_ptr_deref(UnaryOperator *p) {
    auto *builder = _cs->_builder;
    set_current_debug_location(p);

    auto *rhs = p->get_rhs();
    Value *val = codegen(rhs);
    TAN_ASSERT(val->getType()->isPointerTy());

    /// load only if the pointer itself is an lvalue, so that the value after deref is always an lvalue
    if (rhs->get_type()->is_lvalue()) {
      p->_llvm_value = builder->CreateLoad(val, "ptr_deref");
    }

    return p->_llvm_value;
  }

  [[noreturn]] void report_error(ASTBase *p, const str &message) {
    tanlang::report_error(_cs->_filename, _sm->get_token(p->get_loc()), message);
  }
};

CodeGenerator::CodeGenerator(CompilerSession *cs, ASTContext *ctx) { _impl = new CodeGeneratorImpl(cs, ctx); }

llvm::Value *CodeGenerator::codegen(ASTBase *p) { return _impl->codegen(p); }

CodeGenerator::~CodeGenerator() { delete _impl; }

}
