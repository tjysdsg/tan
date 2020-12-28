#include "src/ast/ast_func.h"
#include "src/ast/ast_ty.h"
#include "src/analysis/type_system.h"
#include "parser.h"
#include "reader.h"
#include "token.h"
#include "compiler_session.h"

using namespace tanlang;

Value *ASTFunction::_codegen(CompilerSession *cs) {
  auto *builder = cs->_builder;
  cs->set_current_debug_location(_token->l, _token->c);
  Metadata *ret_meta = _children[0]->to_llvm_meta(cs);
  /// generate prototype
  Function *F = (Function *) codegen_prototype(cs);

  cs->push_scope(_scope); /// push scope
  /// set function arg types
  vector<Metadata *> arg_metas;
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    auto type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_metas.push_back(type_name->to_llvm_meta(cs));
  }

  /// get function name
  str func_name = this->get_name();
  /// function implementation
  if (!_is_external) {
    /// create a new basic block to start insertion into
    BasicBlock *main_block = BasicBlock::Create(*cs->get_context(), "func_entry", F);
    builder->SetInsertPoint(main_block);

    /// debug information
    DIScope *di_scope = cs->get_current_di_scope();
    auto *di_file = cs->get_di_file();
    auto *di_func_t = create_function_type(cs, ret_meta, arg_metas);
    DISubprogram *subprogram = cs->_di_builder
        ->createFunction(di_scope,
            func_name,
            func_name,
            di_file,
            (unsigned) _token->l + 1,
            di_func_t,
            (unsigned) _token->l + 1,
            DINode::FlagPrototyped,
            DISubprogram::SPFlagDefinition,
            nullptr,
            nullptr,
            nullptr);
    F->setSubprogram(subprogram);
    cs->push_di_scope(subprogram);
    /// reset debug emit location
    builder->SetCurrentDebugLocation(DebugLoc());

    /// add all function arguments to scope
    size_t i = 2;
    for (auto &a : F->args()) {
      auto arg_name = _children[i]->get_name();
      auto *arg_val = _children[i]->codegen(cs);
      builder->CreateStore(&a, arg_val);
      /// create a debug descriptor for the arguments
      auto *arg_meta = ast_cast<ASTTy>(_children[i]->_children[1])->to_llvm_meta(cs);
      llvm::DILocalVariable *di_arg = cs->_di_builder
          ->createParameterVariable(subprogram,
              arg_name,
              (unsigned) i + 1,
              di_file,
              (unsigned) _token->l + 1,
              (DIType *) arg_meta,
              true);
      cs->_di_builder
          ->insertDeclare(arg_val,
              di_arg,
              cs->_di_builder->createExpression(),
              llvm::DebugLoc::get((unsigned) _token->l + 1, (unsigned) _token->c + 1, subprogram),
              builder->GetInsertBlock());
      ++i;
    }

    /// set debug emit location to function body
    builder->SetCurrentDebugLocation(DebugLoc::get((unsigned) _children[_children.size() - 1]->_token->l + 1,
        (unsigned) _children[_children.size() - 1]->_token->c + 1,
        subprogram));
    /// generate function body
    _children[_children.size() - 1]->codegen(cs);

    /// create a return instruction if there is none, the return value is the default value of the return type
    if (!builder->GetInsertBlock()->back().isTerminator()) {
      auto ret_ty = _children[0]->get_ty();
      if (ret_ty->_tyty == Ty::VOID) {
        builder->CreateRetVoid();
      } else {
        auto *ret_val = _children[0]->get_llvm_value(cs);
        TAN_ASSERT(ret_val);
        builder->CreateRet(ret_val);
      }
    }
    cs->pop_di_scope();
    /// restore parent code block
    builder->SetInsertPoint(main_block);
  }
  cs->pop_scope(); /// pop scope
  return nullptr;
}

Value *ASTFunction::codegen_prototype(CompilerSession *compiler_session, bool import) {
  Type *ret_type = _children[0]->to_llvm_type(compiler_session);
  vector<Type *> arg_types;
  /// set function arg types
  for (size_t i = 2; i < _children.size() - !_is_external; ++i) {
    auto type_name = ast_cast<ASTTy>(_children[i]->_children[1]);
    arg_types.push_back(type_name->to_llvm_type(compiler_session));
  }
  /// create function prototype
  FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
  auto linkage = Function::InternalLinkage;
  if (_is_external) { linkage = Function::ExternalWeakLinkage; }
  if (_is_public) {
    if (import) {
      linkage = Function::ExternalWeakLinkage;
    } else {
      linkage = Function::ExternalLinkage;
    }
  }
  _func = Function::Create(FT, linkage, this->get_name(), compiler_session->get_module());
  _func->setCallingConv(llvm::CallingConv::C);

  /// set argument names
  auto args = _func->args().begin();
  for (size_t i = 2, j = 0; i < _children.size() - !_is_external; ++i, ++j) {
    /// rename this since the real function argument is stored as variables
    (args + j)->setName("_" + _children[i]->get_name());
  }
  return _func;
}

ASTFunctionPtr ASTFunction::GetCallee(CompilerSession *cs, const str &name, const vector<ASTNodePtr> &args) {
  ASTFunctionPtr ret = nullptr;
  auto func_candidates = cs->get_functions(name);
  /// always prefer the function with lowest cost if multiple candidates are callable
  /// one implicit cast -> +1 cost
  /// however, if two (or more) functions have the same score, an error is raise (ambiguous call)
  auto cost = (size_t) -1;
  for (const auto &f : func_candidates) {
    size_t n = f->get_n_args();
    if (n != args.size()) { continue; }
    bool good = true;
    size_t c = 0;
    for (size_t i = 0; i < n; ++i) { /// check every argument (return type not checked)
      auto arg = f->get_arg(i);
      TAN_ASSERT(arg->_is_typed);
      auto actual_arg = args[i];
      /// allow implicit cast from actual_arg to arg, but not in reverse
      auto t1 = arg->_ty;
      auto t2 = actual_arg->_ty;
      if (*t1 != *t2) {
        if (0 != TypeSystem::CanImplicitCast(cs, t1, t2)) {
          good = false;
          break;
        }
        ++c;
      }
    }
    if (good) {
      if (c < cost) {
        ret = f;
        cost = c;
      } else if (c == cost) { error(cs, "Ambiguous function call: " + name); }
    }
  }
  if (!ret) { error(cs, "Unknown function call: " + name); }
  return ret;
}

ASTFunctionPtr ASTFunction::CreateExtern(const str &name, vector<ASTTyPtr> types) {
  TAN_ASSERT(!types.empty());
  auto ret = std::make_shared<ASTFunction>();
  ret->_name = name;
  ret->_children.reserve(types.size() + 1);
  ret->_children.push_back(types[0]);
  if (types.size() > 1) {
    ret->_children.insert(ret->_children.end(), types.begin() + 1, types.end());
  }
  ret->_parsed = true;
  ret->_is_external = true;
  ret->_is_public = false;
  ret->_name = name;
  return ret;
}
