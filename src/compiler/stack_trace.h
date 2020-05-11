#ifndef __TAN_SRC_COMPILER_STACK_TRACE_H__
#define __TAN_SRC_COMPILER_STACK_TRACE_H__

namespace tanlang {

class CompilerSession;

void codegen_print_stack_trace(CompilerSession *cs);

} // namespace tanlang

#endif //__TAN_SRC_COMPILER_STACK_TRACE_H__
