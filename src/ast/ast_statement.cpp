#include "parser.h"
#include "src/ast/ast_statement.h"
#include "src/ast/astnode.h"
#include <llvm/IR/Instruction.h>
#include <llvm/ADT/APInt.h>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Verifier.h>

namespace tanlang {
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::APFloat;
using llvm::APInt;
using llvm::Type;
using llvm::IRBuilder;
using llvm::AllocaInst;
using llvm::Function;
using llvm::FunctionType;
using llvm::BasicBlock;
using llvm::verifyFunction;

ASTStatement::ASTStatement(bool is_compound, Token *token) : ASTNode(ASTType::STATEMENT,
                                                                     op_precedence[ASTType::STATEMENT],
                                                                     0,
                                                                     token) {
  _is_compound = is_compound;
}
ASTStatement::ASTStatement(Token *token) : ASTNode(ASTType::STATEMENT,
                                                   op_precedence[ASTType::STATEMENT],
                                                   0,
                                                   token) {
}

ASTProgram::ASTProgram() : ASTNode(ASTType::PROGRAM, op_precedence[ASTType::PROGRAM], 0, nullptr) {}

/**
 * \brief: parse a list of (compound) statements
 * */
void ASTProgram::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  while(parser->_curr_token < n_tokens) {
    _children.push_back(parser->next_node());
  }
}

/**
 * \brief: parse a statement if _is_compound is false, otherwise parse a list of (compound) statements and add them
 *          to _children.
 * */
void ASTStatement::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  if (_is_compound) {
    while (parser->_curr_token < n_tokens) {
      auto n = parser->next_statement();
      if (!n || n->_children.empty()) { break; }
      _children.push_back(n);
      ++parser->_curr_token;
    }
  } else {
    auto n = std::reinterpret_pointer_cast<ASTStatement>(parser->next_statement());
    if (n && !n->_children.empty()) {
      *this = *n;
      ++parser->_curr_token;
    }
  }
}

Value *ASTProgram::codegen(ParserContext *parser_context) {
  for (const auto &e : _children) {
    e->codegen(parser_context);
  }
  return nullptr;
}

}
