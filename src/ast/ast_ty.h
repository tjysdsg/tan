#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include "src/ast/astnode.h"

#define TY_GET_BASE(t) ((Ty)((uint64_t)t & TY_BASE_MASK))
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
  INT,
  FLOAT,
  DOUBLE,
  STRING,
  FUNC_PTR, // TODO: function ptr
  STRUCT,   // struct (or class)
  ARRAY,

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
};

class ASTTy final : public ASTNode {
public:
  ASTTy() = delete;
  explicit ASTTy(Token *token);
  void nud(Parser *parser) override;

  [[nodiscard]] llvm::Type *to_llvm_type(CompilerSession *compiler_session) const;

public:
  std::string _type_name{};
private:
  Ty _ty = Ty::INVALID;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
