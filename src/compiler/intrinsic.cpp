#include "intrinsic.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_ty.h"
#include "src/compiler/stack_trace.h"
#include "token.h"

namespace tanlang {

std::unordered_map<str, IntrinsicType>
    Intrinsic::intrinsics{{"abort", IntrinsicType::ABORT}, {"asm", IntrinsicType::ASM}, {"swap", IntrinsicType::SWAP},
    {"memset", IntrinsicType::MEMSET}, {"memcpy", IntrinsicType::MEMCPY}, {"range", IntrinsicType::RANGE},
    {"compile_print", IntrinsicType::COMP_PRINT}, {"file", IntrinsicType::FILENAME}, {"line", IntrinsicType::LINENO},
    {"define", IntrinsicType::DEFINE}, {"sizeof", IntrinsicType::SIZE_OF}, {"offsetof", IntrinsicType::OFFSET_OF},
    {"isa", IntrinsicType::ISA}, {"alignof", IntrinsicType::ALIGN_OF}, {"min_of", IntrinsicType::MIN_OF},
    {"max_of", IntrinsicType::MAX_OF}, {"is_unsigned", IntrinsicType::IS_UNSIGNED},
    {"unlikely", IntrinsicType::UNLIKELY}, {"likely", IntrinsicType::LIKELY}, {"expect", IntrinsicType::EXPECT},
    {"noop", IntrinsicType::NOOP}, {"get_decl", IntrinsicType::GET_DECL}, {"stack_trace", IntrinsicType::STACK_TRACE}};

static void init_noop(CompilerSession *cs);
static void init_abort(CompilerSession *cs);

/// add codegen for function definition if a new function-like intrinsic is added
void Intrinsic::InitCodegen(CompilerSession *cs) {
  init_noop(cs);
  init_abort(cs);
}

Value *Intrinsic::codegen(CompilerSession *cs) {
  ASTNodePtr tmp = nullptr;
  switch (_intrinsic_type) {
    case IntrinsicType::FILENAME:
      tmp = std::make_shared<ASTStringLiteral>(_parser->get_filename(), _start_index);
      _llvm_value = tmp->codegen(cs);
      break;
    case IntrinsicType::LINENO:
      tmp = std::make_shared<ASTNumberLiteral>(_token->l, _start_index);
      _llvm_value = tmp->codegen(cs);
      break;
    case IntrinsicType::GET_DECL:
      tmp = std::make_shared<ASTStringLiteral>(cs->get(_str_data)->get_src(), _start_index);
      _llvm_value = tmp->codegen(cs);
      break;
    case IntrinsicType::STACK_TRACE: {
      codegen_print_stack_trace(cs);
      break;
    }
    default:
      TAN_ASSERT(_children.size());
      _llvm_value = _children[0]->codegen(cs);
      break;
  }
  return _llvm_value;
}

Intrinsic::Intrinsic(Token *token, size_t token_index) : ASTNode(ASTType::INTRINSIC, 0, 0, token, token_index) {
  _start_index = token_index + 1; /// skip "@"
}

void Intrinsic::parse_get_decl() {
  ++_start_index; /// skip self
  _parser->peek(_start_index, TokenType::PUNCTUATION, "(");
  ++_start_index;
  auto input = _parser->next_expression(_start_index);
  if (input->_type != ASTType::ID) { report_code_error(_token, "Invalid call of @get_decl"); }
  TAN_ASSERT(input->is_named());
  _str_data = input->get_name();
  _parser->peek(_start_index, TokenType::PUNCTUATION, ")");
  ++_start_index;
}

void Intrinsic::determine_type() {
  auto *token = _parser->at(_start_index);
  if (Intrinsic::intrinsics.find(token->value) == Intrinsic::intrinsics.end()) {
    report_code_error(_token, "Invalid intrinsic");
  }
  _intrinsic_type = Intrinsic::intrinsics[token->value];
  ASTNodePtr underlying_ast = nullptr;
  switch (_intrinsic_type) {
    case IntrinsicType::NOOP:
      _is_typed = false;
      underlying_ast = std::make_shared<ASTFunctionCall>(token, _start_index);
      _ty = nullptr;
      break;
    case IntrinsicType::ABORT:
      _is_typed = false;
      underlying_ast = std::make_shared<ASTFunctionCall>(token, _start_index);
      _ty = nullptr;
      break;
    case IntrinsicType::STACK_TRACE:
      underlying_ast = std::make_shared<ASTFunctionCall>(token, _start_index);
      break;
    case IntrinsicType::LINENO:
      _ty = ASTTy::Create(TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED), std::vector<ASTNodePtr>());
      break;
    case IntrinsicType::FILENAME:
      _ty = ASTTy::Create(Ty::STRING, std::vector<ASTNodePtr>());
      break;
    case IntrinsicType::GET_DECL:
      _ty = ASTTy::Create(Ty::STRING, std::vector<ASTNodePtr>());
      parse_get_decl();
      break;
    default:
      report_code_error(token, "Unknown intrinsic");
  }
  if (underlying_ast) {
    _lbp = underlying_ast->_lbp;
    _rbp = underlying_ast->_rbp;
    _children.push_back(underlying_ast);
  }
}

size_t Intrinsic::nud() {
  determine_type();
  if (_children.size() >= 1) {
    TAN_ASSERT(_children.size() == 1);
    _end_index = _children[0]->parse(_parser, _cs);
  } else {
    _end_index = _start_index + 1;
  }
  return _end_index;
}

size_t Intrinsic::led(const ASTNodePtr &left) {
  determine_type();
  if (_children.size() >= 1) {
    _end_index = _children[0]->parse(left, _parser, _cs);
  } else {
    _end_index = _start_index + 1;
  }
  return _end_index;
}

str Intrinsic::to_string(bool print_prefix) const {
  str ret;
  if (print_prefix) {
    ret = ASTNode::to_string(print_prefix) + " ";
  }
  if (_children.size() != 0) {
    ret += _children[0]->to_string(false);
  }
  return ret;
}

static void init_abort(CompilerSession *cs) {
  Function *abort_func = cs->get_module()->getFunction("abort");
  /// fn abort() : void;
  if (!abort_func) {
    Type *ret_type = cs->get_builder()->getVoidTy();
    std::vector<Type *> arg_types{};
    FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
    Intrinsic::abort_function = Function::Create(FT, Function::ExternalWeakLinkage, "abort", cs->get_module());
  }
}

static void init_noop(CompilerSession *cs) {
  Function *func = cs->get_module()->getFunction("llvm.donothing");
  if (!func) {
    /// fn llvm.donothing() : void;
    Type *ret_type = cs->get_builder()->getVoidTy();
    std::vector<Type *> arg_types{};
    FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
    Function::Create(FT, Function::ExternalLinkage, "llvm.donothing", cs->get_module());
  }
}

bool Intrinsic::is_lvalue() const { return _is_lvalue; }

bool Intrinsic::is_typed() const { return _is_typed; }

} // namespace tanlang
