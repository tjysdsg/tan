#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include "src/ast/astnode.h"

#define TY_HAS_BASE(t) ((bool)((uint64_t)t & TY_BASE_MASK))
#define TY_HAS_COMPOSITE(t) ((bool)((uint64_t)t & TY_COMPOSITE_MASK))
#define TY_HAS_QUALIFIER(t) ((bool)((uint64_t)t & TY_QUALIFIER_MASK))

#define TY_GET_BASE(t) ((Ty)((uint64_t)t & TY_BASE_MASK))
#define TY_GET_COMPOSITE(t) ((Ty)((uint64_t)t & TY_COMPOSITE_MASK))
#define TY_GET_QUALIFIER(t) ((Ty)((uint64_t)t & TY_QUALIFIER_MASK))

#define TY_IS(t1, t2) ((bool)((uint64_t)(t1) & (uint64_t)(t2)))

#define TY_OR(a, b) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b))
#define TY_OR3(a, b, c) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b) | (uint64_t) (c))

namespace tanlang {

enum class Ty : uint64_t {
  INVALID = 0,
  /// basic types 1->12 bits
  #define TY_BASE_MASK 0xfffu
  VOID = 1u,
  INT = 1u << 1u,
  FLOAT = 1u << 2u,
  DOUBLE = 1u << 3u,
  STRING = 1u << 5u,
  FUNC_PTR = 1u << 6u, // TODO: function ptr

  /// qualifiers 13->32 bits
  #define TY_QUALIFIER_MASK 0xffffff000u

  UNSIGNED = 1u << 13u,
  CONST = 1u << 14u,
  POINTER = 1u << 15u,
  BIT8 = 1u << 16u,
  BIT16 = 1u << 17u,
  BIT32 = 1u << 18u,
  BIT64 = 1u << 19u,
  BIT128 = 1u << 20u,

  /// composite types 33->? bits
  #define TY_COMPOSITE_MASK 0xffffff000000000ull
  STRUCT = 1ull << 33u,   // struct (or class)
  ARRAY = 1ull << 34u,
};

class ASTTy final : public ASTNode {
 public:
  ASTTy() = delete;
  explicit ASTTy(Token *token);
  void nud(Parser *parser) override;

  [[nodiscard]] llvm::Type *to_llvm_type(CompilerSession *compiler_session) const;

 private:
  Ty _ty = Ty::INVALID;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
