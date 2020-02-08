#include "ast.h"
#include "reader.h"
#include "parser.h"
#include "compiler.h"
#include <iostream>
#include "src/llvm_include.h"
#include <gflags/gflags.h>

using tanlang::Reader;
using tanlang::Parser;
using tanlang::Token;

DEFINE_string(files, "main.tan", "comma-separated list of files to compile");
DEFINE_string(output, "output.o", "output file path");

int main(int argc, char **argv) {
  gflags::SetUsageMessage("tan compiler");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // Read code
  Reader r;
  r.open(FLAGS_files); // FIXME: multiple files
  auto tokens = tokenize(&r);
  Parser p(tokens);
  p.parse();
  // p._root->printTree();
  p.codegen();
  // p.get_compiler_session()->get_module()->print(llvm::errs(), nullptr);

  tanlang::Compiler compiler(std::shared_ptr<llvm::Module>(p.get_compiler_session()->get_module().release()));
  compiler.emit_object(FLAGS_output);

  for (auto *&t : tokens) {
    delete t;
    t = nullptr;
  }
  return 0;
}
