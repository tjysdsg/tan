#include "base.h"
#include "token.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"

using tanlang::Reader;
using tanlang::Parser;
using tanlang::Compiler;
using tanlang::Token;

extern Parser *init(const uint8_t *data, size_t size) {
  std::string code = "";
  for (size_t i = 0; i < size; ++i) {
    code.push_back((char) data[i]);
  }
  auto *reader = new Reader;
  reader->from_string(code);
  auto tokens = tokenize(reader);
  auto parser = new Parser(tokens);
  parser->parse();
  return parser;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::cout << size << '\r';
  auto parser = init(data, size);
  parser->codegen();
  // auto compiler = std::make_unique<Compiler>(parser->get_compiler_session()->get_module().release());
  // compiler->emit_object("tmp.o");
  return 0;
}
