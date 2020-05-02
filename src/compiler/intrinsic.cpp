#include "intrinsic.h"
#include "compiler_session.h"
#include "parser.h"
#include "src/ast/ast_string_literal.h"
#include "src/ast/ast_number_literal.h"
#include "src/ast/ast_identifier.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_ty.h"
#include "stack_trace.h"
#include "token.h"

namespace tanlang {

bool Intrinsic::assert_initialized = false;

std::unordered_map<std::string, IntrinsicType>
    Intrinsic::intrinsics{{"abort", IntrinsicType::ABORT}, {"asm", IntrinsicType::ASM}, {"swap", IntrinsicType::SWAP},
    {"memset", IntrinsicType::MEMSET}, {"memcpy", IntrinsicType::MEMCPY}, {"range", IntrinsicType::RANGE},
    {"compile_print", IntrinsicType::COMP_PRINT}, {"file", IntrinsicType::FILENAME}, {"line", IntrinsicType::LINENO},
    {"define", IntrinsicType::DEFINE}, {"sizeof", IntrinsicType::SIZE_OF}, {"offsetof", IntrinsicType::OFFSET_OF},
    {"isa", IntrinsicType::ISA}, {"alignof", IntrinsicType::ALIGN_OF}, {"min_of", IntrinsicType::MIN_OF},
    {"max_of", IntrinsicType::MAX_OF}, {"is_signed", IntrinsicType::IS_SIGNED}, {"unlikely", IntrinsicType::UNLIKELY},
    {"likely", IntrinsicType::LIKELY}, {"expect", IntrinsicType::EXPECT}, {"noop", IntrinsicType::NOOP},
    {"get_decl", IntrinsicType::GET_DECL}, {"stack_trace", IntrinsicType::STACK_TRACE}};

static void init_noop(CompilerSession *compiler_session);
static void init_assert(CompilerSession *compiler_session);

/// add codegen for function definition if a new function-like intrinsic is added
void Intrinsic::InitCodegen(CompilerSession *compiler_session) {
  init_noop(compiler_session);
  init_stack_trace_intrinsic(compiler_session);
  init_assert(compiler_session);
}

llvm::Value *Intrinsic::codegen(CompilerSession *compiler_session) {
  ASTNodePtr tmp = nullptr;
  switch (_intrinsic_type) {
    case IntrinsicType::FILENAME:
      tmp = std::make_shared<ASTStringLiteral>(_parser->get_filename(), _start_index);
      _llvm_value = tmp->codegen(compiler_session);
      break;
    case IntrinsicType::LINENO:
      tmp = std::make_shared<ASTNumberLiteral>(_token->l, _start_index);
      _llvm_value = tmp->codegen(compiler_session);
      break;
    case IntrinsicType::GET_DECL:
      tmp = std::make_shared<ASTStringLiteral>(compiler_session->get(_str_data)->get_src(), _start_index);
      _llvm_value = tmp->codegen(compiler_session);
      break;
    case IntrinsicType::STACK_TRACE: {
      TAN_ASSERT(_children[0]->_type == ASTType::FUNC_CALL);
      TAN_ASSERT(_children[0]->_children[0]->_type == ASTType::NUM_LITERAL);
      auto arg = ast_cast<ASTNumberLiteral>(_children[0]->_children[0]);
      TAN_ASSERT(!arg->is_float());
      _llvm_value = codegen_get_stack_trace(compiler_session, (size_t) (arg->_ivalue));
      break;
    }
    default:
      TAN_ASSERT(_children.size());
      _llvm_value = _children[0]->codegen(compiler_session);
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
    case IntrinsicType::STACK_TRACE:
      // TODO: set _ty
      _is_lvalue = true;
      underlying_ast = std::make_shared<ASTFunctionCall>(token, _start_index);
      break;
    case IntrinsicType::LINENO:
      _ty = ASTTy::Create(TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED));
      break;
    case IntrinsicType::FILENAME:
      _ty = ASTTy::Create(Ty::STRING);
      break;
    case IntrinsicType::GET_DECL:
      _ty = ASTTy::Create(Ty::STRING);
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

llvm::Function *Intrinsic::GetIntrinsic(IntrinsicType type, CompilerSession *compiler_session) {
  Function *f = nullptr;
  switch (type) {
    case IntrinsicType::NOOP:
      f = compiler_session->get_module()->getFunction("llvm.donothing");
      break;
    case IntrinsicType::STACK_TRACE:
      f = compiler_session->get_module()->getFunction("stack_trace");
      break;
    default:
      TAN_ASSERT(false);
      break;
  }
  TAN_ASSERT(f);
  return f;
}

std::string Intrinsic::to_string(bool print_prefix) const {
  std::string ret;
  if (print_prefix) {
    ret = ASTNode::to_string(print_prefix) + " ";
  }
  if (_children.size() != 0) {
    ret += _children[0]->to_string(false);
  }
  return ret;
}

static void init_assert(CompilerSession *compiler_session) {
  if (Intrinsic::assert_initialized) { return; }
  Function *assert_func = compiler_session->get_module()->getFunction("__assert");
  { /// fn __assert(msg: char*, file: char*, line: int) : void;
    if (!assert_func) {
      Type *ret_type = compiler_session->get_builder()->getVoidTy();
      std::vector<Type *> arg_types
          {compiler_session->get_builder()->getInt8PtrTy(), compiler_session->get_builder()->getInt8PtrTy(),
              compiler_session->get_builder()->getInt32Ty(),};
      FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
      assert_func =
          Function::Create(FT, Function::ExternalWeakLinkage, "__assert", compiler_session->get_module().get());
    }
  }
  { /// fn assert(a: i1) : void;
    Function *func = compiler_session->get_module()->getFunction("assert");
    if (!func) {
      Type *ret_type = compiler_session->get_builder()->getVoidTy();
      std::vector<Type *> arg_types{compiler_session->get_builder()->getInt1Ty()};
      FunctionType *func_t = FunctionType::get(ret_type, arg_types, false);
      func = Function::Create(func_t, Function::ExternalLinkage, "assert", compiler_session->get_module().get());
      /// body
      BasicBlock *main_block = BasicBlock::Create(*compiler_session->get_context(), "func_entry", func);
      compiler_session->get_builder()->SetInsertPoint(main_block);
      Value *arg = &(*func->args().begin());
      Value *z = ConstantInt::get(compiler_session->get_builder()->getInt1Ty(), 0, false);
      Value *condition = compiler_session->get_builder()->CreateICmpEQ(z, arg);

      BasicBlock *then_bb = BasicBlock::Create(*compiler_session->get_context(), "then", func);
      BasicBlock *merge_bb = BasicBlock::Create(*compiler_session->get_context(), "fi");

      compiler_session->get_builder()->CreateCondBr(condition, then_bb, merge_bb);
      compiler_session->get_builder()->SetInsertPoint(then_bb);
      auto *st = codegen_get_stack_trace(compiler_session, 1);
      auto *int_t = compiler_session->get_builder()->getInt32Ty();
      auto *zero = ConstantInt::get(int_t, 0);
      auto *one = ConstantInt::get(int_t, 1);
      auto *two = ConstantInt::get(int_t, 2);
      std::vector<Value *> args =
          {compiler_session->get_builder()->CreateLoad(compiler_session->get_builder()->CreateGEP(st, {zero, one})),
              compiler_session->get_builder()->CreateLoad(compiler_session->get_builder()->CreateGEP(st, {zero, zero})),
              compiler_session->get_builder()->CreateLoad(compiler_session->get_builder()->CreateGEP(st, {zero, two}))};
      compiler_session->get_builder()->CreateCall(assert_func, args);
      compiler_session->get_builder()->CreateBr(merge_bb);
      func->getBasicBlockList().push_back(merge_bb);
      compiler_session->get_builder()->SetInsertPoint(merge_bb);
      compiler_session->get_builder()->CreateRetVoid();
    }
  }
  Intrinsic::assert_initialized = true;
}

static void init_noop(CompilerSession *compiler_session) {
  Function *func = compiler_session->get_module()->getFunction("llvm.donothing");
  if (!func) {
    /// fn llvm.donothing() : void;
    Type *ret_type = compiler_session->get_builder()->getVoidTy();
    std::vector<Type *> arg_types{};
    FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
    Function::Create(FT, Function::ExternalLinkage, "llvm.donothing", compiler_session->get_module().get());
  }
}

void Intrinsic::RuntimeInit(CompilerSession *cm) {
  /// stack trace
  runtime_init_stack_trace(cm);
}

llvm::Value *Intrinsic::get_llvm_value(CompilerSession *) const { return _llvm_value; }

bool Intrinsic::is_lvalue() const { return _is_lvalue; }

bool Intrinsic::is_typed() const { return _is_typed; }

std::string Intrinsic::get_type_name() const {
  TAN_ASSERT(_is_typed);
  TAN_ASSERT(_ty);
  return _ty->get_type_name();
}

llvm::Type *Intrinsic::to_llvm_type(CompilerSession *cm) const {
  TAN_ASSERT(_is_typed);
  TAN_ASSERT(_ty);
  return _ty->to_llvm_type(cm);
}

std::shared_ptr<ASTTy> Intrinsic::get_ty() const {
  TAN_ASSERT(_is_typed);
  TAN_ASSERT(_ty);
  return _ty;
}

} // namespace tanlang
