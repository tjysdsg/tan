#include <gflags/gflags.h>
DEFINE_string(files, "main.tan", "comma-separated list of files to compile");
DEFINE_string(output, "output.o", "output file path");
DEFINE_bool(print_ir_code, false, "print out llvm IR code if true");
DEFINE_bool(print_ast, false, "print out abstract syntax tree if true");
using tanlang::Reader;
using tanlang::Parser;
using tanlang::Token;
using tanlang::Compiler;

template<typename PARSER_TYPE>
App<PARSER_TYPE>::App(int argc, char **argv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::ParseCommandLineFlags(&argc, &argv, false);
  _print_ast = FLAGS_print_ast;
  _print_ir_code = FLAGS_print_ir_code;
  _output_file = FLAGS_output;
  std::string delimiter = ",";
  size_t last = 0;
  size_t next = 0;
  while ((next = FLAGS_files.find(delimiter, last)) != std::string::npos) {
    _input_files.push_back(FLAGS_files.substr(last, next - last));
    last = next + 1;
  }
  _input_files.push_back(FLAGS_files.substr(last));
}

template<typename PARSER_TYPE>
bool App<PARSER_TYPE>::read() {
  if (_curr_file >= _input_files.size()) return false;
  Reader r;
  r.open(_input_files[_curr_file]); // FIXME: multiple files
  _tokens = tokenize(&r);
  return true;
}

template<typename PARSER_TYPE>
bool App<PARSER_TYPE>::parse() {
  if (_curr_file >= _input_files.size()) return false;
  _parser = std::make_unique<PARSER_TYPE>(_tokens);
  _parser->parse();
  if (_print_ast) {
    _parser->_root->printTree();
  }
  // _parser->evaluate(); // FIXME: 'main' symbol evaluated the main function in tan-jit.cpp, causing infinite loop
  return true;
}

template<typename PARSER_TYPE>
bool App<PARSER_TYPE>::compile() {
  if (_curr_file >= _input_files.size()) return false;
  _parser->codegen();
  if (_print_ir_code) {
    _parser->get_compiler_session()->get_module()->print(llvm::errs(), nullptr);
  }
  _compiler = std::make_unique<Compiler>(_parser->get_compiler_session()->get_module().release());
  _compiler->emit_object(FLAGS_output);
  return true;
}

template<typename PARSER_TYPE>
App<PARSER_TYPE>::~App() {
  for (auto *&t : _tokens) {
    delete t;
    t = nullptr;
  }
}
