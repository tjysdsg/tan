#include "src/compiler/stack_trace.h"
#include "compiler_session.h"

static llvm::Function *tan_print_bt_func = nullptr;

namespace tanlang {

void codegen_print_stack_trace(CompilerSession *cs) {
  if (tan_print_bt_func) { return; }
  auto *b = cs->get_builder();
  auto *f_t = FunctionType::get(b->getVoidTy(), {});
  tan_print_bt_func = Function::Create(f_t, GlobalValue::ExternalWeakLinkage, "tan_print_st", cs->get_module());
  b->CreateCall(tan_print_bt_func);
}

} // namespace tanlang
