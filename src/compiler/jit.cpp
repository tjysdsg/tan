#include "src/compiler/jit.h"

// FIXME: segfault

namespace tanlang {

JIT::JIT(std::vector<Token *> tokens) : Parser(std::move(tokens)) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  auto context = std::make_unique<LLVMContext>();
  auto builder = std::make_unique<IRBuilder<>>(*context);
  auto module = std::make_unique<Module>("JIT", *context);

  // jit related
  auto execution_session = std::make_unique<ExecutionSession>();
  auto object_layer = std::make_unique<RTDyldObjectLinkingLayer>(
      *execution_session, []() { return std::make_unique<SectionMemoryManager>(); });
  auto jit_machine_builder = JITTargetMachineBuilder::detectHost();
  if (jit_machine_builder.takeError()) {
    throw std::runtime_error("Failed to build JIT machine");
  }
  auto pdata_layout = jit_machine_builder->getDefaultDataLayoutForTarget();
  if (pdata_layout.takeError()) {
    throw std::runtime_error("Failed to get machine data layout");
  }
  auto data_layout = std::make_unique<DataLayout>(pdata_layout.get());
  module->setDataLayout(*data_layout);
  auto mangle = std::make_unique<MangleAndInterner>(*execution_session, *data_layout);

  auto compile_layer =
      std::make_unique<IRCompileLayer>(*execution_session, *object_layer,
                                       ConcurrentIRCompiler(jit_machine_builder.get()));

  execution_session->createJITDylib("<jit-main>");
  auto ctx = std::make_unique<ThreadSafeContext>(std::move(context)); // NOTE context is now nullptr
  execution_session->getMainJITDylib().setGenerator(
      cantFail(DynamicLibrarySearchGenerator::GetForCurrentProcess(data_layout->getGlobalPrefix())));
  _compiler_session = new CompilerSession(std::move(builder),
                                          std::move(module),
                                          std::move(execution_session),
                                          std::move(object_layer),
                                          std::move(compile_layer),
                                          std::move(data_layout),
                                          std::move(mangle),
                                          std::move(ctx));
}

Expected<JITEvaluatedSymbol> JIT::lookup(StringRef name) {
  llvm::ArrayRef<JITDylib *> search_order = {&_compiler_session->get_execution_session()->getMainJITDylib()};
  auto symbol = (*_compiler_session->get_mangle())(name);
  return _compiler_session->get_execution_session()->lookup(search_order, symbol);
}

Error JIT::evaluate(std::unique_ptr<Module> module) {
  if (!module) {
    module = std::move(_compiler_session->get_module());
  }
  auto e = _compiler_session->get_compile_layer()->add(
      _compiler_session->get_execution_session()->getMainJITDylib(),
      ThreadSafeModule(std::move(module), *_compiler_session->get_threadsafe_context())
  );
  _compiler_session->get_execution_session()->dump(llvm::errs());
  if (e) {
    throw std::runtime_error("JIT evaluation failed");
  }
  auto main_func_symbol = lookup("jit_main"); // main function is renamed to 'jit_main' in ast_func.cpp
  if (main_func_symbol.takeError()) {
    throw std::runtime_error("Cannot find main function");
  }
  auto *fp = (float (*)(int, char **)) (intptr_t) main_func_symbol.get().getAddress();
  assert(fp && "Failed to generate code for main function");
  float result = fp(0, nullptr);
  fprintf(stdout, "Main function returned %f\n", result);
  return e;
}

} // namespace tanlang
