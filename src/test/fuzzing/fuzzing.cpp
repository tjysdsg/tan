#include "base.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  using tanlang::Reader;
  using tanlang::Parser;
  using tanlang::Compiler;
  std::string code = "";
  for (size_t i = 0; i < size; ++i) {
    code.push_back((char) data[i]);
  }
  auto reader = std::make_unique<Reader>();
  reader->from_string(code);
  auto tokens = tokenize(reader.get());
  auto parser = std::make_unique<Parser>(tokens);
  parser->parse();
  parser->codegen();
  auto compiler = std::make_unique<Compiler>(parser->get_compiler_session()->get_module().release());
  compiler->emit_object("tmp.o");
  return 0;
}
