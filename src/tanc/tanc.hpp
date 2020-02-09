#include "reader.h"
#include "parser.h"
#include "compiler.h"
using tanlang::Reader;
using tanlang::Parser;

template<typename PARSER_TYPE>
TanC<PARSER_TYPE>::TanC(std::vector<std::string> files, bool print_ast, bool print_ir_code) {
  _print_ast = print_ast;
  _print_ir_code = print_ir_code;
  _input_files = files;
}

template<typename PARSER_TYPE>
bool TanC<PARSER_TYPE>::read() {
  if (_curr_file >= _input_files.size()) return false;
  Reader r;
  r.open(_input_files[_curr_file]);
  _tokens = tokenize(&r);
  return true;
}

template<typename PARSER_TYPE>
bool TanC<PARSER_TYPE>::parse() {
  if (_curr_file >= _input_files.size()) return false;
  _parser = std::make_unique<PARSER_TYPE>(_tokens);
  _parser->parse();
  if (_print_ast) {
    _parser->_root->printTree();
  }
  return true;
}

template<typename PARSER_TYPE>
bool TanC<PARSER_TYPE>::compile() {
  std::cout << "Compiling TAN file: " << _input_files[_curr_file] << "\n";
  if (_curr_file >= _input_files.size()) return false;
  _parser->codegen();
  if (_print_ir_code) {
    _parser->get_compiler_session()->get_module()->print(llvm::errs(), nullptr);
  }
  _parser->evaluate();
  if constexpr (std::is_same<PARSER_TYPE, Parser>::value) { // only compile to file if JIT is disabled
    _compiler = std::make_unique<Compiler>(_parser->get_compiler_session()->get_module().release());
    _compiler->emit_object(_input_files[_curr_file] + ".o");
  }
  return true;
}

template<typename PARSER_TYPE>
TanC<PARSER_TYPE>::~TanC() {
  for (auto *&t : _tokens) {
    delete t;
    t = nullptr;
  }
}

