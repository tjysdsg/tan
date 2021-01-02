#include "intrinsic.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/ast/factory.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_ty.h"
#include "src/compiler/stack_trace.h"
#include "token.h"
#include <iostream>

namespace tanlang {

umap<str, IntrinsicType>
    Intrinsic::intrinsics{{"abort", IntrinsicType::ABORT}, {"asm", IntrinsicType::ASM}, {"swap", IntrinsicType::SWAP},
    {"memset", IntrinsicType::MEMSET}, {"memcpy", IntrinsicType::MEMCPY}, {"range", IntrinsicType::RANGE},
    {"compprint", IntrinsicType::COMP_PRINT}, {"file", IntrinsicType::FILENAME}, {"line", IntrinsicType::LINENO},
    {"define", IntrinsicType::DEFINE}, {"sizeof", IntrinsicType::SIZE_OF}, {"offsetof", IntrinsicType::OFFSET_OF},
    {"isa", IntrinsicType::ISA}, {"alignof", IntrinsicType::ALIGN_OF}, {"min_of", IntrinsicType::MIN_OF},
    {"max_of", IntrinsicType::MAX_OF}, {"is_unsigned", IntrinsicType::IS_UNSIGNED},
    {"unlikely", IntrinsicType::UNLIKELY}, {"likely", IntrinsicType::LIKELY}, {"expect", IntrinsicType::EXPECT},
    {"noop", IntrinsicType::NOOP}, {"get_decl", IntrinsicType::GET_DECL}, {"stack_trace", IntrinsicType::STACK_TRACE}};

static void init_noop(CompilerSession *cs);
static void init_abort(CompilerSession *cs);

void Intrinsic::InitAnalysis(CompilerSession *cs) {
  cs->add_function(ASTFunction::CreateExtern("compprint", {create_ty(cs, Ty::VOID), create_ty(cs, Ty::STRING)}));
}

/// add _codegen for function definition if a new function-like intrinsic is added
void Intrinsic::InitCodegen(CompilerSession *cs) {
  init_noop(cs);
  init_abort(cs);
}

Value *Intrinsic::_codegen(CompilerSession *cs) {
  ASTNodePtr tmp = nullptr;
  switch (_intrinsic_type) {
    case IntrinsicType::FILENAME:
      auto c = ast_create_string_literal(cs, cs->_filename);
      break;
    case IntrinsicType::LINENO:
      tmp = std::make_shared<ASTNumberLiteral>(_token->l, _start_index);
      _llvm_value = tmp->codegen(cs);
      break;
    case IntrinsicType::GET_DECL:
      tmp = std::make_shared<ASTStringLiteral>(cs->get(_str_data)->get_src(), _start_index);
      _llvm_value = tmp->codegen(cs);
      break;
    case IntrinsicType::STACK_TRACE:
      codegen_print_stack_trace(cs);
      break;
    case IntrinsicType::COMP_PRINT:
      break;
    default:
      TAN_ASSERT(_children.size());
      _llvm_value = _children[0]->codegen(cs);
      break;
  }
  return _llvm_value;
}

static void init_abort(CompilerSession *cs) {
  Function *abort_func = cs->get_module()->getFunction("abort");
  /// fn abort() : void;
  if (!abort_func) {
    Type *ret_type = cs->_builder->getVoidTy();
    vector<Type *> arg_types{};
    FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
    Intrinsic::abort_function = Function::Create(FT, Function::ExternalWeakLinkage, "abort", cs->get_module());
  }
}

static void init_noop(CompilerSession *cs) {
  Function *func = cs->get_module()->getFunction("llvm.donothing");
  if (!func) {
    /// fn llvm.donothing() : void;
    Type *ret_type = cs->_builder->getVoidTy();
    vector<Type *> arg_types{};
    FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
    Function::Create(FT, Function::ExternalLinkage, "llvm.donothing", cs->get_module());
  }
}

} // namespace tanlang
