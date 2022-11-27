#include "ast/intrinsic.h"
#include "compiler/compiler_session.h" // TODO IMPORTANT: remove this dependency
#include "ast/decl.h"
#include "ast/type.h"
#include "ast/ast_context.h"
#include "llvm_api/llvm_include.h"

namespace tanlang {

umap<str, IntrinsicType> Intrinsic::intrinsics{
    {"asm",             IntrinsicType::ASM            },
    {"swap",            IntrinsicType::SWAP           },
    {"memset",          IntrinsicType::MEMSET         },
    {"memcpy",          IntrinsicType::MEMCPY         },
    {"range",           IntrinsicType::RANGE          },
    {"compprint",       IntrinsicType::COMP_PRINT     },
    {"file",            IntrinsicType::FILENAME       },
    {"line",            IntrinsicType::LINENO         },
    {"define",          IntrinsicType::DEFINE         },
    {"sizeof",          IntrinsicType::SIZE_OF        },
    {"offsetof",        IntrinsicType::OFFSET_OF      },
    {"isa",             IntrinsicType::ISA            },
    {"alignof",         IntrinsicType::ALIGN_OF       },
    {"min_of",          IntrinsicType::MIN_OF         },
    {"max_of",          IntrinsicType::MAX_OF         },
    {"is_unsigned",     IntrinsicType::IS_UNSIGNED    },
    {"unlikely",        IntrinsicType::UNLIKELY       },
    {"likely",          IntrinsicType::LIKELY         },
    {"expect",          IntrinsicType::EXPECT         },
    {"noop",            IntrinsicType::NOOP           },
    {"get_decl",        IntrinsicType::GET_DECL       },
    {"test_comp_error", IntrinsicType::TEST_COMP_ERROR},
    {"stack_trace",     IntrinsicType::STACK_TRACE    }
};

static void init_noop(CompilerSession *cs);

void Intrinsic::InitAnalysis(ASTContext *ctx) {
  /// @compprint
  ctx->add_function(
      FunctionDecl::Create(SrcLoc(0), "compprint", Type::GetVoidType(), {Type::GetStringType()}, true, false));
}

Intrinsic *Intrinsic::Create(SrcLoc loc) { return new Intrinsic(loc); }

Intrinsic::Intrinsic(SrcLoc loc) : Expr(ASTNodeType::INTRINSIC, loc, 0) {}

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

} // namespace tanlang
