#include "ast/intrinsic.h"
#include "ast/decl.h"
#include "ast/type.h"

namespace tanlang {

umap<str, IntrinsicType> Intrinsic::INTRINSIC_NAME_TO_TYPES{
    {"asm",                          IntrinsicType::ASM            },
    {"swap",                         IntrinsicType::SWAP           },
    {"memset",                       IntrinsicType::MEMSET         },
    {"memcpy",                       IntrinsicType::MEMCPY         },
    {"range",                        IntrinsicType::RANGE          },
    {COMP_PRINT_NAME,                IntrinsicType::COMP_PRINT     },
    {"file",                         IntrinsicType::FILENAME       },
    {"line",                         IntrinsicType::LINENO         },
    {"define",                       IntrinsicType::DEFINE         },
    {"sizeof",                       IntrinsicType::SIZE_OF        },
    {"offsetof",                     IntrinsicType::OFFSET_OF      },
    {"isa",                          IntrinsicType::ISA            },
    {"alignof",                      IntrinsicType::ALIGN_OF       },
    {"min_of",                       IntrinsicType::MIN_OF         },
    {"max_of",                       IntrinsicType::MAX_OF         },
    {"is_unsigned",                  IntrinsicType::IS_UNSIGNED    },
    {"unlikely",                     IntrinsicType::UNLIKELY       },
    {"likely",                       IntrinsicType::LIKELY         },
    {"expect",                       IntrinsicType::EXPECT         },
    {"noop",                         IntrinsicType::NOOP           },
    {"get_decl",                     IntrinsicType::GET_DECL       },
    {TEST_COMP_ERROR_NAME,           IntrinsicType::TEST_COMP_ERROR},
    {ABORT_NAME,                     IntrinsicType::ABORT          },
    {"stack_trace",                  IntrinsicType::STACK_TRACE    },
    {STACK_TRACE_FUNCTION_REAL_NAME, IntrinsicType::STACK_TRACE    }
};

vector<FunctionDecl *> Intrinsic::GetIntrinsicFunctionDeclarations() {
  vector<FunctionDecl *> ret{};
  auto *src = new TokenizedSourceFile("__intrinsics__", {});

  // compprint
  ret.push_back(FunctionDecl::Create(src, COMP_PRINT_NAME,
                                     Type::GetFunctionType(Type::GetVoidType(), {Type::GetStringType()}), true, false,
                                     nullptr, true));

  // abort
  ret.push_back(FunctionDecl::Create(src, ABORT_NAME, Type::GetFunctionType(Type::GetVoidType(), {}), true, false,
                                     nullptr, true));

  return ret;
}

Intrinsic *Intrinsic::Create(TokenizedSourceFile *src) { return new Intrinsic(src); }

Intrinsic::Intrinsic(TokenizedSourceFile *src) : Expr(ASTNodeType::INTRINSIC, src, 0) {}

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

str Intrinsic::terminal_token() const {
  if (_intrinsic_type == IntrinsicType::TEST_COMP_ERROR) {
    return "}";
  }

  return ";";
}

} // namespace tanlang
