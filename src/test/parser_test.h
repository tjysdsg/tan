#ifndef TAN_LEXER_TEST_H
#define TAN_LEXER_TEST_H

#include "lexer.h"
#include "token.h"
#include "compiler_session.h"
#include "reader.h"
#include "parser.h"
#include "src/ast/ast_node.h"
#include <gtest/gtest.h>
#include <iostream>

using tanlang::Reader;
using tanlang::Parser;
using tanlang::CompilerSession;
using tanlang::tokenize;
using tanlang::TokenType;
using tanlang::ASTNodePtr;
using tanlang::ASTType;

ASTNodePtr parse_string(str code) {
  Reader reader;
  reader.from_string(code);
  auto result = tokenize(&reader);
  auto tokens = tokenize(&reader);

  llvm::TargetMachine *tm = nullptr;
  { // FIXME: remove this
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    auto target_triple = llvm::sys::getDefaultTargetTriple();
    str error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    /// relocation model
    auto RM = llvm::Reloc::Model::PIC_;
    tm = target->createTargetMachine(target_triple, CPU, features, opt, RM);
  }

  CompilerSession *cs = new CompilerSession("test module", tm);
  auto *parser = new Parser(tokens, "test", cs);
  return parser->parse();
}

TEST(parser, function_decl) {
  str code = "pub fn greet_cj(is_cj: bool, big_smoke: str) : i32 {"
             "  // you picked the wrong house, fool\n"
             "  return 666;"
             "}";
  auto node = parse_string(code);
  EXPECT_EQ(ASTType::FUNC_DECL, node->_type);
}

#endif /* TAN_LEXER_TEST_H */
