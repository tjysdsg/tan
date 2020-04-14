#ifndef __TAN_INCLUDE_STACK_TRACE_H__
#define __TAN_INCLUDE_STACK_TRACE_H__
#include <string>
#include <memory>

namespace llvm {
class Value;
}

namespace tanlang {

class CompilerSession;

struct StackTrace {
  std::string _filename = "";
  std::string _src = "";
  size_t _lineno = 0;
};

llvm::Value *codegen_push_stack_trace(CompilerSession *compiler_session, std::shared_ptr<StackTrace> stack_trace);
void codegen_pop_stack_trace(CompilerSession *compiler_session);
llvm::Value *codegen_get_stack_trace(CompilerSession *compiler_session, size_t level = 0);

}

#endif /* __TAN_INCLUDE_STACK_TRACE_H__ */
