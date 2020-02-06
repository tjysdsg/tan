#ifndef TAN_SRC_COMPILER_JIT_H_
#define TAN_SRC_COMPILER_JIT_H_
#include <memory>
#include "parser.h"
#include "src/llvm_include.h"

namespace tanlang {

/*
class JIT : public Parser {
 private:
  ExecutionSession _execution_session;
  RTDyldObjectLinkingLayer _object_layer;
  IRCompileLayer _compile_layer;

  DataLayout _data_layout;
  MangleAndInterner *_mangle;
  ThreadSafeContext _ctx;

  JITDylib &_main_jd;

 public:
  JIT(std::vector<Token *> tokens)
      : Parser(tokens), _object_layer(_execution_session,
                                      []() { return std::make_unique<SectionMemoryManager>(); }) {
    // ==============================================
    _parser_context = new CompilerSession("JIT");
    auto jit_machine_builder = JITTargetMachineBuilder::detectHost();
    if (!jit_machine_builder) {
      // return jit_machine_builder.takeError();
      return;
    }
    auto data_layout = jit_machine_builder->getDefaultDataLayoutForTarget();
    if (!data_layout) {
      // return data_layout.takeError();
      return;
    }
    _parser_context->_module->setDataLayout(*data_layout);

    _compile_layer.add(_execution_session.getMainJITDylib(),
                       ThreadSafeModule(std::move(_parser_context->_module), _ctx));

    _compile_layer(_execution_session, _object_layer, ConcurrentIRCompiler(std::move(jit_machine_builder)));
    _data_layout = *data_layout;
    _mangle = new MangleAndInterner(_execution_session, _data_layout);
    _ctx = ThreadSafeContext(std::move(_parser_context->_context)));
    _main_jd = _execution_session.createJITDylib("<main>");
    _execution_session.getMainJITDylib().setGenerator(
        cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(data_layout.getGlobalPrefix())));
  }

  const DataLayout &get_data_layout() const { return _data_layout; }

  void add_module(std::unique_ptr<Module> module) {
    cantFail(_compile_layer.add(_execution_session.getMainJITDylib(), ThreadSafeModule(std::move(module), _ctx)));
  }

  Expected<JITEvaluatedSymbol> lookup(StringRef Name) {
    return _execution_session.lookup({&_execution_session.getMainJITDylib()}, _mangle(Name.str()));
  }
};
 */

} // namespace tanlang

#endif /* TAN_SRC_COMPILER_JIT_H_ */
