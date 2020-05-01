#include "stack_trace.h"
#include "src/llvm_include.h"
#include "intrinsic.h"
#include "compiler_session.h"

static llvm::Type *g_stack_trace_t = nullptr;
static llvm::Value *g_stack_trace = nullptr;

namespace tanlang {

void runtime_init_stack_trace(CompilerSession *compiler_session) {
  auto *st_t = g_stack_trace_t;
  auto *init = ConstantPointerNull::get(st_t->getPointerTo());
  {
    GlobalVariable *tmp = compiler_session->get_module()->getNamedGlobal("st");
    TAN_ASSERT(tmp);
    tmp->setExternallyInitialized(false);
    tmp->setInitializer(init);
    g_stack_trace = tmp;
  }
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  Value *st = compiler_session->get_builder()->CreateAlloca(st_t, ConstantInt::get(int_t, MAX_N_FUNCTION_CALLS, false));
  compiler_session->get_builder()->CreateStore(st, g_stack_trace);
}

void init_stack_trace_intrinsic(CompilerSession *compiler_session) {
  StructType *st_t = get_stack_trace_type(compiler_session);
  /// a global pointer to an array of StackTrace
  GlobalVariable *st = new GlobalVariable(*compiler_session->get_module(),
      st_t->getPointerTo(),
      false,
      GlobalValue::ExternalWeakLinkage,
      nullptr,
      "st");
  st->setExternallyInitialized(true);
  g_stack_trace = st;
}

llvm::Value *codegen_push_stack_trace(CompilerSession *compiler_session, std::shared_ptr<StackTrace> stack_trace) {
  auto *st = compiler_session->get_builder()->CreateLoad(g_stack_trace);
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  auto *one = ConstantInt::get(int_t, 1);

  auto *filename = compiler_session->get_builder()->CreateGlobalStringPtr(stack_trace->_filename);
  auto *src = compiler_session->get_builder()->CreateGlobalStringPtr(stack_trace->_src);
  auto *lineno = ConstantInt::get(int_t, stack_trace->_lineno);
  auto *val = ConstantStruct::get((StructType *) g_stack_trace_t, {filename, src, lineno});
  compiler_session->get_builder()->CreateStore(val, st);

  compiler_session->get_builder()->CreateStore(compiler_session->get_builder()->CreateGEP(st, one), g_stack_trace);
  return st;
}

void codegen_pop_stack_trace(CompilerSession *compiler_session) {
  auto *st = compiler_session->get_builder()->CreateLoad(g_stack_trace);
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  auto *tmp = compiler_session->get_builder()->CreateGEP(st, ConstantInt::get(int_t, (uint64_t) (-1), true));
  compiler_session->get_builder()->CreateStore(tmp, g_stack_trace);
}

llvm::Value *codegen_get_stack_trace(CompilerSession *compiler_session, size_t level) {
  Value *st = compiler_session->get_builder()->CreateLoad(g_stack_trace);
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  st = compiler_session->get_builder()->CreateGEP(st, ConstantInt::get(int_t, (uint64_t) (-level), true));
  return st;
}

StructType *get_stack_trace_type(CompilerSession *compiler_session) {
  if (g_stack_trace_t) { return (StructType *) g_stack_trace_t; }
  llvm::StructType *struct_type = llvm::StructType::create(*compiler_session->get_context(), "StackTrace");
  struct_type->setBody(compiler_session->get_builder()->getInt8PtrTy(),
      compiler_session->get_builder()->getInt8PtrTy(),
      compiler_session->get_builder()->getInt32Ty());
  g_stack_trace_t = struct_type;
  return struct_type;
}

} // namespace tanlang
