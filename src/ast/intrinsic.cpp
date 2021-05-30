#include "src/ast/intrinsic.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/ast/factory.h"
#include "src/ast/decl.h"
#include "src/ast/ast_type.h"
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
  cs->add_function(FunctionDecl::Create("compprint",
      ASTType::Create(cs, Ty::VOID),
      {ASTType::Create(cs, Ty::STRING),},
      true,
      false));
}

void Intrinsic::InitCodegen(CompilerSession *cs) {
  init_noop(cs);
  init_abort(cs);
}

ptr<Intrinsic> Intrinsic::Create() { return make_ptr<Intrinsic>(); }

Intrinsic::Intrinsic() : ASTBase(ASTNodeType::INTRINSIC, 0) {}

void Intrinsic::set_sub(ExprPtr sub) { _sub = sub; }

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