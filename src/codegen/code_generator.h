#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#include "base.h"

namespace llvm {
class Value;
}

namespace tanlang {

class CodeGeneratorImpl;

class CodeGenerator {
public:
  CodeGenerator() = delete;
  ~CodeGenerator();
  explicit CodeGenerator(CompilerSession *cs);
  llvm::Value *codegen(ASTBase *p);

private:
  CodeGeneratorImpl *_impl = nullptr;
};

}

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_H__
