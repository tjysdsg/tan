#include "intrinsic.h"
#include "compiler_session.h"
#include "parser.h"

namespace tanlang {

std::unordered_map<std::string, IntrinsicType>
    Intrinsic::intrinsics
    {{"assert", IntrinsicType::ASSERT}, {"abort", IntrinsicType::ABORT}, {"asm", IntrinsicType::ASM},
     {"swap", IntrinsicType::SWAP}, {"memset", IntrinsicType::MEMSET}, {"memcpy", IntrinsicType::MEMCPY},
     {"range", IntrinsicType::RANGE}, {"cast", IntrinsicType::CAST}, {"compile_print", IntrinsicType::COMP_PRINT},
     {"__FILE__", IntrinsicType::FILENAME}, {"__LINE__", IntrinsicType::LINENO}, {"define", IntrinsicType::DEFINE},
     {"sizeof", IntrinsicType::SIZE_OF}, {"offsetof", IntrinsicType::OFFSET_OF}, {"isa", IntrinsicType::ISA},
     {"alignof", IntrinsicType::ALIGN_OF}, {"min_of", IntrinsicType::MIN_OF}, {"max_of", IntrinsicType::MAX_OF},
     {"is_signed", IntrinsicType::IS_SIGNED}, {"__UNLIKELY__", IntrinsicType::UNLIKELY},
     {"__LIKELY__", IntrinsicType::LIKELY}, {"expect", IntrinsicType::EXPECT}, {"__noop__", IntrinsicType::NOOP}};

static llvm::Value *init_noop(CompilerSession *compiler_session);
static llvm::Value *init_assert(CompilerSession *compiler_session);

/// add codegen for function definition if a new function-like intrinsic is added
void Intrinsic::InitCodegen(CompilerSession *compiler_session) {
  init_assert(compiler_session);
  init_noop(compiler_session);
}

llvm::Value *Intrinsic::codegen(CompilerSession *compiler_session) {
  _llvm_value = _underlying_ast->codegen(compiler_session);
  return _llvm_value;
}

Intrinsic::Intrinsic(std::string filename, Token *token, size_t token_index) : ASTNode(ASTType::INTRINSIC,
                                                                                       0,
                                                                                       0,
                                                                                       token,
                                                                                       token_index
) {
  _intrinsic_type = Intrinsic::intrinsics[token->value];
  switch (_intrinsic_type) {
    case IntrinsicType::ASSERT:
      _underlying_ast = std::make_shared<ASTFunctionCall>(token, token_index);
      break;
    case IntrinsicType::NOOP:
      _underlying_ast = std::make_shared<ASTFunctionCall>(token, token_index);
      break;
    case IntrinsicType::LINENO:
      _underlying_ast = std::make_shared<ASTNumberLiteral>(token->l + 1, token_index); /// token->l starts at 0
      break;
    case IntrinsicType::FILENAME:
      _underlying_ast = std::make_shared<ASTStringLiteral>(filename, token_index);
      break;
      // TODO: other intrinsics
    default:
      report_code_error(token, "Unknown intrinsic");
  }
  _lbp = _underlying_ast->_lbp;
  _rbp = _underlying_ast->_rbp;
}

size_t Intrinsic::parse(Parser *parser) {
  return _underlying_ast->parse(parser);
}

size_t Intrinsic::parse(const std::shared_ptr<ASTNode> &left, Parser *parser) {
  assert(_underlying_ast);
  return _underlying_ast->parse(left, parser);
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

static llvm::Value *init_assert(CompilerSession *compiler_session) {
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
  return nullptr;
}

static llvm::Value *init_noop(CompilerSession *compiler_session) {
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
  return nullptr;
}
} // namespace tanlang
