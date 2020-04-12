#include "intrinsic.h"
#include "compiler_session.h"
#include "parser.h"

namespace tanlang {

std::unordered_map<std::string, IntrinsicType>
    Intrinsic::intrinsics
    {{"assert", IntrinsicType::ASSERT}, {"abort", IntrinsicType::ABORT}, {"asm", IntrinsicType::ASM},
     {"swap", IntrinsicType::SWAP}, {"memset", IntrinsicType::MEMSET}, {"memcpy", IntrinsicType::MEMCPY},
     {"range", IntrinsicType::RANGE}, {"cast", IntrinsicType::CAST}, {"compile_print", IntrinsicType::COMP_PRINT},
     {"file", IntrinsicType::FILENAME}, {"line", IntrinsicType::LINENO}, {"define", IntrinsicType::DEFINE},
     {"sizeof", IntrinsicType::SIZE_OF}, {"offsetof", IntrinsicType::OFFSET_OF}, {"isa", IntrinsicType::ISA},
     {"alignof", IntrinsicType::ALIGN_OF}, {"min_of", IntrinsicType::MIN_OF}, {"max_of", IntrinsicType::MAX_OF},
     {"is_signed", IntrinsicType::IS_SIGNED}, {"unlikely", IntrinsicType::UNLIKELY}, {"likely", IntrinsicType::LIKELY},
     {"expect", IntrinsicType::EXPECT}, {"noop", IntrinsicType::NOOP}, {"get_decl", IntrinsicType::GET_DECL},
     {"stack_trace", IntrinsicType::STACK_TRACE}};

static void init_noop(CompilerSession *compiler_session);
static void init_assert(CompilerSession *compiler_session);
static llvm::Type *get_stack_trace_type(CompilerSession *compiler_session);

/// add codegen for function definition if a new function-like intrinsic is added
void Intrinsic::InitCodegen(CompilerSession *compiler_session) {
  init_assert(compiler_session);
  init_noop(compiler_session);
}

Value *get_stack_trace(CompilerSession *compiler_session) {
  // TODO: show full stacktrace by settings _caller
  // struct StackTrace {
  //   var filename: str;
  //   var src: str;
  //   var lineno: u32;
  //   var caller: StackTrace*;
  // };
  auto *st_t = get_stack_trace_type(compiler_session);
  auto *filename =
      compiler_session->get_builder()->CreateGlobalStringPtr(compiler_session->get_stack_trace()->_filename);
  auto *src = compiler_session->get_builder()->CreateGlobalStringPtr(compiler_session->get_stack_trace()->_src);
  auto *lineno = llvm::ConstantInt::get(compiler_session->get_builder()->getInt32Ty(),
                                        compiler_session->get_stack_trace()->_lineno,
                                        false
  );
  return llvm::ConstantStruct::get((llvm::StructType *) st_t, {filename, src, lineno});
}

llvm::Value *Intrinsic::codegen(CompilerSession *compiler_session) {
  std::shared_ptr<ASTNode> tmp = nullptr;
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
    case IntrinsicType::STACK_TRACE:
      _llvm_value = get_stack_trace(compiler_session);
      break;
    default:
      assert(_children.size());
      _llvm_value = _children[0]->codegen(compiler_session);
      break;
  }
  return _llvm_value;
}

Intrinsic::Intrinsic(Token *token, size_t token_index) : ASTNode(ASTType::INTRINSIC, 0, 0, token, token_index
) {
  _start_index = token_index + 1; /// skip "@"
}

void Intrinsic::parse_get_decl() {
  ++_start_index; /// skip self
  _parser->peek(_start_index, TokenType::PUNCTUATION, "(");
  ++_start_index;
  auto input = _parser->next_expression(_start_index);
  if (input->_type != ASTType::ID) {
    report_code_error(_token, "Invalid call of @get_decl");
  }
  _str_data = ast_cast<ASTIdentifier>(input)->get_name();
  _parser->peek(_start_index, TokenType::PUNCTUATION, ")");
  ++_start_index;
}

void Intrinsic::determine_type() {
  auto *token = _parser->at(_start_index);
  if (Intrinsic::intrinsics.find(token->value) == Intrinsic::intrinsics.end()) {
    report_code_error(_token, "Invalid intrinsic");
  }
  _intrinsic_type = Intrinsic::intrinsics[token->value];
  std::shared_ptr<ASTNode> underlying_ast = nullptr;
  switch (_intrinsic_type) {
    case IntrinsicType::ASSERT:
    case IntrinsicType::NOOP:
      underlying_ast = std::make_shared<ASTFunctionCall>(token, _start_index);
      break;
    case IntrinsicType::STACK_TRACE:
    case IntrinsicType::LINENO:
    case IntrinsicType::FILENAME:
      break;
    case IntrinsicType::GET_DECL:
      parse_get_decl();
      break;
      // TODO: other intrinsics
    default:
      report_code_error(token, "Unknown intrinsic");
  }
  if (underlying_ast) {
    _lbp = underlying_ast->_lbp;
    _rbp = underlying_ast->_rbp;
    _children.push_back(underlying_ast);
  }
}

size_t Intrinsic::parse(Parser *parser) {
  _parser = parser;
  determine_type();
  if (_children.size() >= 1) {
    assert(_children.size() == 1);
    _end_index = _children[0]->parse(parser);
  } else {
    _end_index = _start_index + 1;
  }
  return _end_index;
}

size_t Intrinsic::parse(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  _parser = parser;
  determine_type();
  if (_children.size() >= 1) {
    _end_index = _children[0]->parse(left, parser);
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
    case IntrinsicType::ASSERT:
      f = compiler_session->get_module()->getFunction("assert");
      break;
    default:
      assert(false);
      break;
  }
  assert(f);
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
  Function *func = compiler_session->get_module()->getFunction("__assert");
  if (!func) {
    {
      /// extern void __assert(const char *msg, const char *file, int line);
      Type *ret_type = compiler_session->get_builder()->getVoidTy();
      std::vector<Type *> arg_types
          {compiler_session->get_builder()->getInt8PtrTy(), compiler_session->get_builder()->getInt8PtrTy(),
           compiler_session->get_builder()->getInt32Ty(),};
      FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
      Function::Create(FT, Function::ExternalLinkage, "__assert", compiler_session->get_module().get());
    }
  }
}

static void init_noop(CompilerSession *compiler_session) {
  Function *func = compiler_session->get_module()->getFunction("llvm.donothing");
  if (!func) {
    {
      /// extern void llvm.donothing();
      /// extern void __noop__();
      Type *ret_type = compiler_session->get_builder()->getVoidTy();
      std::vector<Type *> arg_types{};
      FunctionType *FT = FunctionType::get(ret_type, arg_types, false);
      Function::Create(FT, Function::ExternalLinkage, "llvm.donothing", compiler_session->get_module().get());
      Function::Create(FT, Function::ExternalLinkage, "__noop__", compiler_session->get_module().get());
    }
  }
}

static llvm::Type *get_stack_trace_type(CompilerSession *compiler_session) {
  // TODO: optimize this
  /// define StackTrace type
  llvm::StructType *struct_type = llvm::StructType::create(*compiler_session->get_context(), "StackTrace");
  struct_type->setBody({compiler_session->get_builder()->getInt8PtrTy(),
                        compiler_session->get_builder()->getInt8PtrTy(), compiler_session->get_builder()->getInt32Ty()}
  );
  return struct_type;
}

} // namespace tanlang
