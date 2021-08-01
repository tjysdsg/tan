#include "src/ast/intrinsic.h"
#include "compiler_session.h"
#include "src/ast/decl.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_context.h"
#include "src/compiler/stack_trace.h"

namespace tanlang {

umap<str, IntrinsicType>
    Intrinsic::intrinsics{{"asm", IntrinsicType::ASM}, {"swap", IntrinsicType::SWAP}, {"memset", IntrinsicType::MEMSET},
    {"memcpy", IntrinsicType::MEMCPY}, {"range", IntrinsicType::RANGE}, {"compprint", IntrinsicType::COMP_PRINT},
    {"file", IntrinsicType::FILENAME}, {"line", IntrinsicType::LINENO}, {"define", IntrinsicType::DEFINE},
    {"sizeof", IntrinsicType::SIZE_OF}, {"offsetof", IntrinsicType::OFFSET_OF}, {"isa", IntrinsicType::ISA},
    {"alignof", IntrinsicType::ALIGN_OF}, {"min_of", IntrinsicType::MIN_OF}, {"max_of", IntrinsicType::MAX_OF},
    {"is_unsigned", IntrinsicType::IS_UNSIGNED}, {"unlikely", IntrinsicType::UNLIKELY},
    {"likely", IntrinsicType::LIKELY}, {"expect", IntrinsicType::EXPECT}, {"noop", IntrinsicType::NOOP},
    {"get_decl", IntrinsicType::GET_DECL}, {"test_comp_error", IntrinsicType::TEST_COMP_ERROR}};

static void init_noop(CompilerSession *cs);

void Intrinsic::InitAnalysis(ASTContext *ctx) {
  /// @compprint
  ctx->add_function(FunctionDecl::Create(SourceIndex(0),
      "compprint",
      ASTType::GetVoidType(ctx, SourceIndex(0)),
      {ASTType::CreateAndResolve(ctx, SourceIndex(0), Ty::STRING),},
      true,
      false));
}

void Intrinsic::InitCodegen(CompilerSession *cs) {
  init_noop(cs);
}

Intrinsic *Intrinsic::Create(SourceIndex loc) { return new Intrinsic(loc); }

Intrinsic::Intrinsic(SourceIndex loc) : Expr(ASTNodeType::INTRINSIC, loc, 0) {}

IntrinsicType Intrinsic::get_intrinsic_type() const { return _intrinsic_type; }

void Intrinsic::set_intrinsic_type(IntrinsicType intrinsic_type) { _intrinsic_type = intrinsic_type; }

ASTBase *Intrinsic::get_sub() const { return _sub; }

void Intrinsic::set_sub(ASTBase *sub) { _sub = sub; }

vector<ASTBase *> Intrinsic::get_children() const {
  if (_sub) {
    return _sub->get_children();
  }
  return {};
}

static void init_noop(CompilerSession *cs) {
  /// fn llvm.donothing() : void;
  Function *func = cs->get_module()->getFunction("llvm.donothing");
  if (!func) {
    Type *ret_type = cs->_builder->getVoidTy();
    FunctionType *FT = FunctionType::get(ret_type, {}, false);
    Function::Create(FT, Function::ExternalWeakLinkage, "llvm.donothing", cs->get_module());
  }
}

} // namespace tanlang
