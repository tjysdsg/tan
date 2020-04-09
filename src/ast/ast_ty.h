#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include "src/ast/astnode.h"

#define TY_GET_BASE(t) ((Ty)((uint64_t)t & TY_BASE_MASK))
#define TY_GET_QUALIFIER(t) ((Ty)((uint64_t)t & TY_QUALIFIER_MASK))

#define TY_IS(t1, t2) ((bool)((uint64_t)(t1) & (uint64_t)(t2)))

#define TY_OR(a, b) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b))
#define TY_OR3(a, b, c) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b) | (uint64_t) (c))

namespace tanlang {
class Parser;

enum class Ty : uint64_t {
  INVALID = 0,
  /// basic types 1->12 bits
  #define TY_BASE_MASK 0xfffu
  VOID = 1u,
  INT,
  FLOAT,
  DOUBLE,
  BOOL,
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

/**
 * Things to remember when adding a new type
 * - set _type_name in ASTTy::nud()
 * - set _llvm_type in ASTTy::codegen()
 */
class ASTTy final : public ASTNode {
public:
  ASTTy() = delete;
  ASTTy(Token *token, size_t token_index);

  bool is_typed() const override { return true; }

  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *compiler_session) const override;
  std::string to_string(bool print_prefix = true) const override;
  llvm::Value *convert_to(CompilerSession *compiler_session, std::shared_ptr<ASTTy> dest_ty, llvm::Value *orig_val);

  void set_is_lvalue(bool heaped) { _is_lvalue = heaped; }

protected:
  size_t nud(Parser *parser) override;
private:
  size_t nud_array(Parser *parser);

private:
  std::string _type_name{};
  Ty _ty = Ty::INVALID;
  size_t _n_elements = 0;
  bool _is_lvalue = false;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
