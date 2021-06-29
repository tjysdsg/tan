#ifndef __TAN_SRC_CODEGEN_CODEGEN_HELPER_H__
#define __TAN_SRC_CODEGEN_CODEGEN_HELPER_H__
#include "src/llvm_include.h"
#include "src/ast/fwd.h"
#include "compiler_session.h"

namespace tanlang {

class CodegenHelper {
public:
  static Constant *CodegenIntegerLiteral(CompilerSession *cs,
      uint64_t val,
      unsigned bit_size,
      bool is_unsigned = false);

};

}

#endif //__TAN_SRC_CODEGEN_CODEGEN_HELPER_H__
