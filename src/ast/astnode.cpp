#include "src/ast/astnode.h"
#include "parser.h"
#include <llvm/ADT/APFloat.h>
#include <llvm/IR/Function.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instruction.h>

// TODO: error reporting
// TODO: implement scope
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

// ================= helper functions ================//
void ASTNode::report_error() {
  report_code_error(_token->l, _token->c, "Unexpected token " + _token->to_string());
}
void ASTNode::printSubtree(const std::string &prefix) const {
  using std::cout;
  using std::endl;
  if (_children.empty()) return;
  cout << prefix;
  size_t n_children = _children.size();
  cout << (n_children > 1 ? "├── " : "");

  for (size_t i = 0; i < n_children; ++i) {
    const auto &c = _children[i];
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
void ASTNode::printTree() const {
  using std::cout;
  std::cout << ast_type_names[this->_op] << "\n";
  printSubtree("");
  cout << "\n";
}

// ====================================================//

// ======================== cdtors =================== //
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

ASTCompare::ASTCompare(ASTType type, Token *token) : ASTInfixBinaryOp(token) {
  // TODO: assert type
  _op = type;
  _lbp = op_precedence[type];
}

ASTReturn::ASTReturn(Token *token) : ASTPrefix(token) {
  _op = ASTType::RET;
  _lbp = op_precedence[_op];
}

ASTNode::ASTNode(ASTType op, int lbp, int rbp, Token *token)
    : _op(op), _lbp(lbp), _rbp(rbp), _token(token) {}

ASTProgram::ASTProgram() : ASTNode(ASTType::PROGRAM, op_precedence[ASTType::PROGRAM], 0, nullptr) {}

ASTInfixBinaryOp::ASTInfixBinaryOp(Token *token) : ASTNode(ASTType::INVALID,
                                                           op_precedence[ASTType::INVALID],
                                                           0,
                                                           token) {}

ASTNumberLiteral::ASTNumberLiteral(const std::string &str, bool is_float, Token *token) : ASTNode(ASTType::NUM_LITERAL,
                                                                                                  op_precedence[ASTType::NUM_LITERAL],
                                                                                                  0, token) {
  _is_float = is_float;
  if (is_float) {
    _fvalue = std::stof(str);
  } else {
    _ivalue = std::stoi(str);
  }
}

ASTPrefix::ASTPrefix(Token *token) : ASTNode(ASTType::INVALID, op_precedence[ASTType::INVALID], 0, token) {}

ASTStringLiteral::ASTStringLiteral(std::string str, Token *token) : ASTNode(ASTType::STRING_LITERAL,
                                                                            op_precedence[ASTType::STRING_LITERAL],
                                                                            0, token), _svalue(std::move(str)) {}

ASTLogicalNot::ASTLogicalNot(Token *token) : ASTPrefix(token) {
  _op = ASTType::LNOT;
  _lbp = op_precedence[_op];
}

ASTBinaryNot::ASTBinaryNot(Token *token) : ASTPrefix(token) {
  _op = ASTType::BNOT;
  _lbp = op_precedence[_op];
}

ASTArithmetic::ASTArithmetic(ASTType type, Token *token) : ASTInfixBinaryOp(token) {
  _op = type;
  _lbp = op_precedence[type];
}

// ============================================================ //

// ============================= parser =========================//
void ASTNode::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  UNUSED(left);
  UNUSED(parser);
  if (!_token) {
    throw std::runtime_error("Unexpected empty token");
  }
  report_error();
}

void ASTNode::nud(Parser *parser) {
  UNUSED(parser);
  if (!_token) {
    throw std::runtime_error("Unexpected empty token");
  }
  report_error();
}

void ASTInfixBinaryOp::led(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _children.emplace_back(left);
  auto n = parser->next_expression(_lbp);
  if (!n) {
    report_error();
  } else {
    _children.emplace_back(n);
  }
}

/**
 * This defined only to overwrite ASTNode::nud() because the latter throws
 * */
void ASTNumberLiteral::nud(Parser *parser) {
  UNUSED(parser);
}

/**
 * \brief: parse a list of (compound) statements
 * */
void ASTProgram::nud(Parser *parser) {
  size_t n_tokens = parser->_tokens.size();
  auto *t = parser->_tokens[parser->_curr_token];
  if (t->type == TokenType::PUNCTUATION && t->value == "{") {
    auto n = parser->advance();
    n->nud(parser);
    _children.push_back(n);
    return;
  }
  while (parser->_curr_token < n_tokens) {
    auto n = std::reinterpret_pointer_cast<ASTStatement>(parser->next_statement());
    if (!n || n->_children.empty()) { break; }
    _children.push_back(n);
    ++parser->_curr_token;
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

void ASTPrefix::nud(Parser *parser) {
  auto n = parser->next_expression(_lbp);
  if (!n) {
    throw std::runtime_error("Expect a token"); // FIXME: improve this error
  } else {
    _children.emplace_back(n);
  }
}
// ==============================================================//

void ASTNode::add(ASTNode *c) {
  _children.emplace_back(c);
}

// ========================== getter/setter ====================//
int ASTNode::get_ivalue() const {
  throw std::runtime_error("NOT IMPLEMENTED");
}

float ASTNode::get_fvalue() const {
  throw std::runtime_error("NOT IMPLEMENTED");
}

std::string ASTNode::get_svalue() const {
  throw std::runtime_error("NOT IMPLEMENTED");
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

std::string ASTStringLiteral::get_svalue() const {
  return _svalue;
}
// ==============================================================//

// ================= codegen functions ========================= //
Value *ASTNode::codegen(ParserContext *parser_context) {
  if (_children.empty()) return nullptr;
  auto *result = _children[0]->codegen(parser_context);
  size_t n = _children.size();
  for (size_t i = 1; i < n; ++i) {
    _children[i]->codegen(parser_context);
  }
  return result;
}

Value *ASTNumberLiteral::codegen(ParserContext *parser_context) {
  if (_is_float) {
    return ConstantFP::get(*parser_context->_context, APFloat(_fvalue));
  } else {
    return ConstantInt::get(*parser_context->_context, APInt(32, static_cast<uint64_t>(_ivalue), true));
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
  auto *rhs = _children[0]->codegen(parser_context);
  if (!rhs) {
    assert(false);
  }
  // get value size in bits
  llvm::DataLayout data_layout(parser_context->_module.get());
  auto size_in_bits = data_layout.getTypeSizeInBits(rhs->getType());
  auto mask = ConstantInt::get(*parser_context->_context,
                               APInt(static_cast<uint32_t>(size_in_bits), std::numeric_limits<uint64_t>::max(), false));
  return parser_context->_builder->CreateXor(mask, rhs);
}

Value *ASTCompare::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  // TODO: handle type conversion
  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();
  Type *float_type = parser_context->_builder->getFloatTy();
  if (ltype->isPointerTy()) {
    lhs = parser_context->_builder->CreateLoad(float_type, lhs);
  }
  if (rtype->isPointerTy()) {
    rhs = parser_context->_builder->CreateLoad(float_type, rhs);
  }
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = parser_context->_builder->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = parser_context->_builder->CreateSIToFP(rhs, float_type);
    }

    if (_op == ASTType::GT) {
      return parser_context->_builder->CreateFCmpOGT(lhs, rhs);
    } else if (_op == ASTType::GE) {
      return parser_context->_builder->CreateFCmpOGE(lhs, rhs);
    } else if (_op == ASTType::LT) {
      return parser_context->_builder->CreateFCmpOLT(lhs, rhs);
    } else if (_op == ASTType::LE) {
      return parser_context->_builder->CreateFCmpOLE(lhs, rhs);
    } else {
      // TODO: report error
      return nullptr;
    }
  }
  if (_op == ASTType::GT) {
    return parser_context->_builder->CreateICmpUGT(lhs, rhs);
  } else if (_op == ASTType::GE) {
    return parser_context->_builder->CreateICmpUGE(lhs, rhs);
  } else if (_op == ASTType::LT) {
    return parser_context->_builder->CreateICmpULT(lhs, rhs);
  } else if (_op == ASTType::LE) {
    return parser_context->_builder->CreateICmpULE(lhs, rhs);
  } else {
    // TODO: report error
    return nullptr;
  }
}

Value *ASTArithmetic::codegen(ParserContext *parser_context) {
  Value *lhs = _children[0]->codegen(parser_context);
  Value *rhs = _children[1]->codegen(parser_context);
  if (!lhs || !rhs) {
    assert(false);
    return nullptr;
  }
  Type *ltype = lhs->getType();
  Type *rtype = rhs->getType();
  Type *float_type = parser_context->_builder->getFloatTy();

  if (ltype->isPointerTy()) {
    lhs = parser_context->_builder->CreateLoad(float_type, lhs);
  }
  if (rtype->isPointerTy()) {
    rhs = parser_context->_builder->CreateLoad(float_type, rhs);
  }
  if (!ltype->isIntegerTy() || !rtype->isIntegerTy()) {
    if (ltype->isIntegerTy()) {
      lhs = parser_context->_builder->CreateSIToFP(lhs, float_type);
    }
    if (rtype->isIntegerTy()) {
      rhs = parser_context->_builder->CreateSIToFP(rhs, float_type);
    }
    // float arithmetic
    if (_op == ASTType::MULTIPLY) {
//      return parser_context->_builder->CreateFMul(lhs, rhs);
      return parser_context->_builder->CreateFMul(lhs, rhs);
    } else if (_op == ASTType::DIVIDE) {
      return parser_context->_builder->CreateFDiv(lhs, rhs);
    } else if (_op == ASTType::SUM) {
      return parser_context->_builder->CreateFAdd(lhs, rhs);
    } else if (_op == ASTType::SUBTRACT) {
      return parser_context->_builder->CreateFSub(lhs, rhs);
    } else {
      // TODO: report error
      return nullptr;
    }
  }

  // integer arithmetic
  if (_op == ASTType::MULTIPLY) {
    return parser_context->_builder->CreateMul(lhs, rhs, "mul_tmp");
  } else if (_op == ASTType::DIVIDE) {
    return parser_context->_builder->CreateUDiv(lhs, rhs, "div_tmp");
  } else if (_op == ASTType::SUM) {
    return parser_context->_builder->CreateAdd(lhs, rhs, "sum_tmp");
  } else if (_op == ASTType::SUBTRACT) {
    return parser_context->_builder->CreateSub(lhs, rhs, "sub_tmp");
  } else {
    // TODO: report error
    return nullptr;
  }
}

Value *ASTReturn::codegen(ParserContext *parser_context) {
  return parser_context->_builder->CreateRet(_children[0]->codegen(parser_context));
}

/**
 * \brief Create an alloca instruction in the entry block of
 * the function. This is used for mutable variables etc.
 */
static AllocaInst *CreateEntryBlockAlloca(Function *func, const std::string &name, ParserContext *parser_context) {
  IRBuilder<> tmp_builder(&func->getEntryBlock(), func->getEntryBlock().begin());
  return tmp_builder.CreateAlloca(parser_context->_builder->getFloatTy(), nullptr, name);
}

Value *ASTProgram::codegen(ParserContext *parser_context) {
  // make function prototype
  Type *float_type = parser_context->_builder->getFloatTy();
  std::vector<Type *> arg_types(2, float_type);
  FunctionType *FT = FunctionType::get(float_type, arg_types, false);
  Function *F = Function::Create(FT, Function::ExternalLinkage, "main", *parser_context->_module);

  // TODO: function argument type
  unsigned Idx = 0;
  for (auto &Arg : F->args()) {
    Arg.setName("arg" + std::to_string(Idx++));
  }

  // function implementation
  // create a new basic block to start insertion into
  BasicBlock *main_block = BasicBlock::Create(*parser_context->_context, "entry", F);
  parser_context->_builder->SetInsertPoint(main_block);

  // TODO: create a new scope
  // Record the function arguments in the NamedValues map.
  for (auto &Arg : F->args()) {
    parser_context->add_variable(Arg.getName(), &Arg);
    // Create an alloca for this variable.
    AllocaInst *alloca = CreateEntryBlockAlloca(F, Arg.getName(), parser_context);
    // Store the initial value into the alloca.
    parser_context->_builder->CreateStore(&Arg, alloca);

    // Add arguments to variable symbol table.
    parser_context->set_variable(Arg.getName(), alloca);
  }

  for (const auto &child : _children) {
    child->codegen(parser_context);
  }
  // validate the generated code, checking for consistency
  verifyFunction(*F);
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
    MAKE_ASTTYPE_NAME_PAIR(IF),
    MAKE_ASTTYPE_NAME_PAIR(ELSE),
    MAKE_ASTTYPE_NAME_PAIR(GT),
    MAKE_ASTTYPE_NAME_PAIR(GE),
    MAKE_ASTTYPE_NAME_PAIR(LT),
    MAKE_ASTTYPE_NAME_PAIR(LE),
    MAKE_ASTTYPE_NAME_PAIR(ID),
    MAKE_ASTTYPE_NAME_PAIR(PARENTHESIS),

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

    {ASTType::PARENTHESIS, PREC_CALL},

    {ASTType::RET, PREC_KEYWORD},
    {ASTType::IF, PREC_KEYWORD},
    {ASTType::ELSE, PREC_KEYWORD},

    {ASTType::BNOT, PREC_UNARY},
    {ASTType::LNOT, PREC_UNARY},

    {ASTType::LAND, PREC_LOGICAL_AND},
    {ASTType::LOR, PREC_LOGICAL_OR},

    {ASTType::NUM_LITERAL, PREC_LITERAL},
    {ASTType::STRING_LITERAL, PREC_LITERAL}
};
} // namespace tanlang
