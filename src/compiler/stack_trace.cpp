#include "stack_trace.h"
#include "src/llvm_include.h"
#include "intrinsic.h"
#include "compiler_session.h"

namespace tanlang {

llvm::Value *codegen_push_stack_trace(CompilerSession *compiler_session, std::shared_ptr<StackTrace> stack_trace) {
  auto *st = compiler_session->get_builder()->CreateLoad(Intrinsic::stack_trace);
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  auto *one = ConstantInt::get(int_t, 1);

  auto *filename = compiler_session->get_builder()->CreateGlobalStringPtr(stack_trace->_filename);
  auto *src = compiler_session->get_builder()->CreateGlobalStringPtr(stack_trace->_src);
  auto *lineno = ConstantInt::get(int_t, stack_trace->_lineno);
  auto *val = ConstantStruct::get((StructType *) Intrinsic::stack_trace_t, {filename, src, lineno});
  compiler_session->get_builder()->CreateStore(val, st);

  compiler_session->get_builder()
      ->CreateStore(compiler_session->get_builder()->CreateGEP(st, one), Intrinsic::stack_trace);
  return st;
}

void codegen_pop_stack_trace(CompilerSession *compiler_session) {
  auto *st = compiler_session->get_builder()->CreateLoad(Intrinsic::stack_trace);
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  auto *tmp = compiler_session->get_builder()->CreateGEP(st, ConstantInt::get(int_t, (uint64_t) (-1), true));
  compiler_session->get_builder()->CreateStore(tmp, Intrinsic::stack_trace);
}

llvm::Value *codegen_get_stack_trace(CompilerSession *compiler_session, size_t level) {
  Value *st = compiler_session->get_builder()->CreateLoad(Intrinsic::stack_trace);
  auto *int_t = compiler_session->get_builder()->getInt32Ty();
  st = compiler_session->get_builder()->CreateGEP(st, ConstantInt::get(int_t, (uint64_t) (-level), true));
  return st;
}

} // namespace tanlang
