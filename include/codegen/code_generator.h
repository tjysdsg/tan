#ifndef __TAN_SRC_CODEGEN_CODE_GENERATOR_H__
#define __TAN_SRC_CODEGEN_CODE_GENERATOR_H__

namespace llvm {
class Value;
}

namespace tanlang {

class CodeGeneratorImpl;
class CompilerSession;
class ASTBase;
class ASTContext;

class CodeGenerator {
public:
  CodeGenerator() = delete;
  ~CodeGenerator();
  explicit CodeGenerator(CompilerSession *cs, ASTContext *ctx);
  llvm::Value *codegen(ASTBase *p);

private:
  CodeGeneratorImpl *_impl = nullptr;
};

}

#endif //__TAN_SRC_CODEGEN_CODE_GENERATOR_H__
