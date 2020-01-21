#include "src/ast/astnode.h"
#include <iostream>
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>

namespace tanlang {
using llvm::ConstantFP;
using llvm::ConstantInt;
using llvm::APFloat;
using llvm::APInt;
using llvm::IRBuilder;

void ASTNode::printSubtree(const std::string &prefix) {
  using std::cout;
  using std::endl;
  if (_children.empty()) return;
  cout << prefix;
  size_t n_children = _children.size();
  cout << (n_children > 1 ? "├── " : "");

  for (size_t i = 0; i < n_children; ++i) {
    ASTNode *c = _children[i];
    if (i < n_children - 1) {
      bool printStrand = n_children > 1 && !c->_children.empty();
      std::string newPrefix = prefix + (printStrand ? "│\t" : "\t");
      std::cout << ast_type_names[c->_op] << "\n";
      c->printSubtree(newPrefix);
    } else {
      cout << (n_children > 1 ? prefix : "") << "└── ";
      std::cout << ast_type_names[c->_op] << "\n";
      c->printSubtree(prefix + "\t");
    }
  }
}

void ASTNode::printTree() {
  using std::cout;
  std::cout << ast_type_names[this->_op] << "\n";
  printSubtree("");
  cout << "\n";
}

ASTStatement::ASTStatement(bool is_compound) {
  _op = ASTType::STATEMENT;
  _lbp = op_precedence[_op];
  _is_compound = is_compound;
}

ASTStatement::ASTStatement() {
  _op = ASTType::STATEMENT;
  _lbp = op_precedence[_op];
}

ASTCompare::ASTCompare(ASTType type) {
  _op = type;
  _lbp = op_precedence[type];
}

ASTReturn::ASTReturn() {
  _op = ASTType::RET;
  _lbp = op_precedence[_op];
}

ASTNode::~ASTNode() {
  for (auto *&c : _children) {
    delete c;
    c = nullptr;
  }
}

ASTNode *ASTNode::led(ASTNode *left, Parser *parser) {
  UNUSED(left);
  UNUSED(parser);
  assert(false);
  return nullptr;
}

ASTNode *ASTNode::nud(Parser *parser) {
  UNUSED(parser);
  assert(false);
  return nullptr;
}

void ASTNode::add(ASTNode *c) {
  _children.emplace_back(c);
}

int ASTNode::get_ivalue() const {
  assert(false);
  return 0;
}

float ASTNode::get_fvalue() const {
  assert(false);
  return 0;
}

std::string ASTNode::get_svalue() const {
  assert(false);
  return "";
}

ASTNode::ASTNode(ASTType op, int associativity, int lbp, int rbp)
    : _op(op), _associativity(associativity), _lbp(lbp), _rbp(rbp) {}

ASTProgram::ASTProgram() {
  _op = ASTType::PROGRAM;
}

ASTNode *ASTInfixBinaryOp::led(ASTNode *left, Parser *parser) {
  _children.emplace_back(left);
  auto *n = parser->next_expression(_lbp);
  if (!n) {
    // TODO: report error
    throw "SHIT";
  } else {
    _children.emplace_back(n);
  }
  return this;
}

ASTNode *ASTInfixBinaryOp::nud(Parser *parser) {
  UNUSED(parser);
  assert(false);
  return nullptr;
}

ASTInfixBinaryOp::ASTInfixBinaryOp() : ASTNode(ASTType::INVALID, 0, op_precedence[ASTType::INVALID], 0) {}

ASTNumberLiteral::ASTNumberLiteral(const std::string &str, bool is_float) : ASTNode(ASTType::NUM_LITERAL, 1,
                                                                                    op_precedence[ASTType::NUM_LITERAL],
                                                                                    0) {
  _is_float = is_float;
  if (is_float) {
    _fvalue = std::stof(str);
  } else {
    _ivalue = std::stoi(str);
  }
}
ASTNode *ASTNumberLiteral::nud(Parser *parser) {
  UNUSED(parser);
  return this;
}

ASTNode *ASTProgram::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  auto *t = parser->_tokens[parser->_curr_token];
  if (t->type == TokenType::PUNCTUATION && t->value == "{") {
    auto *n = parser->advance();
    _children.push_back(n->nud(parser));
    return this;
  }
  while (parser->_curr_token < n_tokens) {
    auto *n = reinterpret_cast<ASTStatement *>(parser->next_statement());
    if (!n || n->_children.empty()) { break; }
    _children.push_back(n);
    ++parser->_curr_token;
  }
  return this;
}

ASTNode *ASTStatement::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  if (_is_compound) {
    while (parser->_curr_token < n_tokens) {
      auto *n = parser->next_statement();
      if (!n || n->_children.empty()) { break; }
      _children.push_back(n);
      ++parser->_curr_token;
    }
  } else {
    auto *n = reinterpret_cast<ASTStatement *>(parser->next_statement());
    if (!n || n->_children.empty()) {
      // TODO: report errors
    } else {
      this->_is_compound = n->_is_compound;
      this->_children = n->_children;
      this->_op = n->_op;
      this->_associativity = n->_associativity;
      this->_lbp = n->_lbp;
      this->_rbp = n->_rbp;
    }
  }
  return this;
}

bool ASTNumberLiteral::is_float() const {
  return _is_float;
}

int ASTNumberLiteral::get_ivalue() const {
  assert(!_is_float);
  return _ivalue;
}

float ASTNumberLiteral::get_fvalue() const {
  assert(_is_float);
  return _fvalue;
}

ASTNode *ASTPrefix::nud(Parser *parser) {
  auto *n = parser->next_expression(_lbp);
  if (!n) {
    // TODO: report error
  } else {
    _children.emplace_back(n);
  }
  return this;
}

ASTPrefix::ASTPrefix() : ASTNode(ASTType::INVALID, 1, op_precedence[ASTType::INVALID], 0) {}

ASTStringLiteral::ASTStringLiteral(std::string str) : ASTNode(ASTType::STRING_LITERAL, 1,
                                                              op_precedence[ASTType::STRING_LITERAL],
                                                              0), _svalue(std::move(str)) {
}

std::string ASTStringLiteral::get_svalue() const {
  return _svalue;
}

ASTLogicalNot::ASTLogicalNot() : ASTPrefix() {
  _op = ASTType::LNOT;
  _lbp = op_precedence[_op];
}

ASTBinaryNot::ASTBinaryNot() : ASTPrefix() {
  _op = ASTType::BNOT;
  _lbp = op_precedence[_op];
}

ASTMultiply::ASTMultiply() : ASTInfixBinaryOp() {
  _op = ASTType::MULTIPLY;
  _lbp = op_precedence[ASTType::MULTIPLY];
}

ASTDivide::ASTDivide() : ASTInfixBinaryOp() {
  _op = ASTType::DIVIDE;
  _lbp = op_precedence[ASTType::DIVIDE];
}

ASTSum::ASTSum() : ASTInfixBinaryOp() {
  _op = ASTType::SUM;
  _lbp = op_precedence[ASTType::SUM];
}

ASTSubtract::ASTSubtract() : ASTInfixBinaryOp() {
  _op = ASTType::SUBTRACT;
  _lbp = op_precedence[ASTType::SUBTRACT];
}

// ================= codegen functions ================ //
Value *ASTNumberLiteral::codegen(ParserContext *parser_context) {
  if (_is_float) {
    return ConstantFP::get(*parser_context->_context, APFloat(_fvalue));
  } else {
    return ConstantInt::get(*parser_context->_context, APInt(32, _ivalue, true));
  }
}

Value *ASTBinaryNot::codegen(ParserContext *parser_context) {
  auto *rhs = _children[0]->codegen(parser_context);
  if (!rhs) {
    assert(false);
  }
  return parser_context->_builder->CreateNot(rhs);
}

Value *ASTLogicalNot::codegen(ParserContext *parser_context) {
  // FIXME
  auto *rhs = _children[0]->codegen(parser_context);
  if (!rhs) {
    assert(false);
  }
  return parser_context->_builder->CreateNot(rhs);
}

Value *ASTCompare::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  // TODO: handle type conversion
  auto *ltype = lhs->getType();
  auto *rtype = rhs->getType();
  if (ltype->isFloatingPointTy() || rtype->isFloatingPointTy()) {
    if (ltype->isIntegerTy()) {
      parser_context->_builder->CreateUIToFP(lhs, rtype);
    }
    if (rtype->isIntegerTy()) {
      parser_context->_builder->CreateUIToFP(rhs, ltype);
    }
  }
  if (_op == ASTType::GT) {
    return parser_context->_builder->CreateICmpUGT(lhs, rhs, "gt_tmp");
  } else if (_op == ASTType::GE) {
    return parser_context->_builder->CreateICmpUGE(lhs, rhs, "ge_tmp");
  } else if (_op == ASTType::LT) {
    return parser_context->_builder->CreateICmpULT(lhs, rhs, "lt_tmp");
  } else if (_op == ASTType::LE) {
    return parser_context->_builder->CreateICmpULE(lhs, rhs, "le_tmp");
  }
}

Value *ASTSum::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  return parser_context->_builder->CreateFAdd(lhs, rhs, "add_tmp");
}

Value *ASTSubtract::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  return parser_context->_builder->CreateFSub(lhs, rhs, "sub_tmp");
}

Value *ASTMultiply::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  return parser_context->_builder->CreateFMul(lhs, rhs, "mul_tmp");
}

Value *ASTDivide::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  return parser_context->_builder->CreateFDiv(lhs, rhs, "div_tmp");
}

Value *ASTReturn::codegen(ParserContext *parser_context) {
  return parser_context->_builder->CreateRet(_children[0]->codegen(parser_context));
}

Value *ASTProgram::codegen(ParserContext *parser_context) {
  using llvm::Function;
  using llvm::FunctionType;
  using llvm::Type;
  using llvm::BasicBlock;
  using llvm::verifyFunction;
  // Make the function type:  double(double,double) etc.
  std::vector<Type *> Doubles(2, Type::getDoubleTy(*parser_context->_context));
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(*parser_context->_context), Doubles, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, "main", *parser_context->_module);

  // TODO: main function arguments
  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName(std::to_string(Idx++));
  }

  // create a new basic block to start insertion into
  BasicBlock *main_block = BasicBlock::Create(*parser_context->_context, "entry", F);
  parser_context->_builder->SetInsertPoint(main_block);

  for (auto *child : _children) {
    child->codegen(parser_context);
  }
  // validate the generated code, checking for consistency
  verifyFunction(*F);
  return nullptr;
}

Value *ASTStatement::codegen(ParserContext *parser_context) {
  for (auto *child : _children) {
    child->codegen(parser_context);
  }
  return nullptr;
}

// ================= codegen functions ends ================ //

#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTType::t, #t}

std::unordered_map<ASTType, std::string> ast_type_names{
    MAKE_ASTTYPE_NAME_PAIR(PROGRAM),
    MAKE_ASTTYPE_NAME_PAIR(STATEMENT),

    MAKE_ASTTYPE_NAME_PAIR(SUM),
    MAKE_ASTTYPE_NAME_PAIR(SUBTRACT),
    MAKE_ASTTYPE_NAME_PAIR(MULTIPLY),
    MAKE_ASTTYPE_NAME_PAIR(DIVIDE),
    MAKE_ASTTYPE_NAME_PAIR(MOD),
    MAKE_ASTTYPE_NAME_PAIR(ASSIGN),

    MAKE_ASTTYPE_NAME_PAIR(NUM_LITERAL),
    MAKE_ASTTYPE_NAME_PAIR(STRING_LITERAL),
    MAKE_ASTTYPE_NAME_PAIR(BAND),
    MAKE_ASTTYPE_NAME_PAIR(LAND),
    MAKE_ASTTYPE_NAME_PAIR(BOR),
    MAKE_ASTTYPE_NAME_PAIR(LOR),
    MAKE_ASTTYPE_NAME_PAIR(BNOT),
    MAKE_ASTTYPE_NAME_PAIR(LNOT),
    MAKE_ASTTYPE_NAME_PAIR(XOR),
    MAKE_ASTTYPE_NAME_PAIR(RET),
};

#undef MAKE_ASTTYPE_NAME_PAIR

// operator precedence for each token
std::unordered_map<ASTType, int> op_precedence{
    {ASTType::PROGRAM, PREC_LOWEST},
    {ASTType::STATEMENT, PREC_LOWEST},
    {ASTType::INVALID, PREC_LOWEST},

    {ASTType::SUM, PREC_TERM},
    {ASTType::SUBTRACT, PREC_TERM},
    {ASTType::BOR, PREC_TERM},
    {ASTType::XOR, PREC_TERM},

    {ASTType::MULTIPLY, PREC_FACTOR},
    {ASTType::DIVIDE, PREC_FACTOR},
    {ASTType::MOD, PREC_FACTOR},
    {ASTType::BAND, PREC_FACTOR},
    {ASTType::GT, PREC_COMPARISON},
    {ASTType::GE, PREC_COMPARISON},
    {ASTType::LT, PREC_COMPARISON},
    {ASTType::LE, PREC_COMPARISON},

    {ASTType::ASSIGN, PREC_ASSIGN},

    {ASTType::RET, PREC_KEYWORD},

    {ASTType::BNOT, PREC_UNARY},
    {ASTType::LNOT, PREC_UNARY},

    {ASTType::LAND, PREC_LOGICAL_AND},
    {ASTType::LOR, PREC_LOGICAL_OR},

    {ASTType::NUM_LITERAL, PREC_LITERAL},
    {ASTType::STRING_LITERAL, PREC_LITERAL}
};
} // namespace tanlang
