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
  INT = 2u,
  FLOAT = 3u,
  DOUBLE = 4u,
  BOOL = 5u,
  POINTER = 6u,
  STRING = 7u,
  CHAR = 8u,
  FUNC_PTR = 9u, // TODO: function ptr
  STRUCT = 10u,   // struct (or class)
  ARRAY = 11u,

  /// qualifiers 13->32 bits
  #define TY_QUALIFIER_MASK 0xffffff000u

  UNSIGNED = 1u << 13u,
  CONST = 1u << 14u,
  BIT8 = 1u << 15u,
  BIT16 = 1u << 16u,
  BIT32 = 1u << 17u,
  BIT64 = 1u << 18u,
  BIT128 = 1u << 19u,
};

class ASTTy;

using ASTTyPtr = std::shared_ptr<ASTTy>;

class ASTTy final : public ASTNode, public std::enable_shared_from_this<ASTTy> {
public:
  static std::shared_ptr<ASTTy> Create(Ty t, bool is_lvalue = false, std::vector<std::shared_ptr<ASTTy>> sub_tys = {});

public:
  ASTTy() = delete;
  ASTTy(Token *token, size_t token_index);

  ASTTyPtr get_contained_ty() const;
  std::string get_type_name() const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  llvm::Type *to_llvm_type(CompilerSession *compiler_session) const override;
  llvm::DIType *to_llvm_meta(CompilerSession *compiler_session) const override;
  std::string to_string(bool print_prefix = true) const override;
  bool operator==(const ASTTy &other) const;
  bool operator!=(const ASTTy &other) const;
  void set_is_lvalue(bool is_lvalue);
  bool is_lvalue() const override;
  bool is_typed() const override;
  size_t get_size_bits() const;
  bool is_ptr() const;
  bool is_float() const;
  bool is_floating() const;
  bool is_double() const;
  bool is_int() const;
  bool is_bool() const;
  bool is_unsigned() const;
  bool is_struct() const;

public:
  Ty _ty = Ty::INVALID;

private:
  size_t nud_array();
  size_t nud() override;
  void resolve();

private:
  mutable std::string _type_name{};
  size_t _size_bits = 0;
  size_t _align_bits = 0;
  unsigned _dwarf_encoding = 0;
  bool _is_ptr = false;
  bool _is_float = false;
  bool _is_double = false;
  bool _is_int = false;
  bool _is_unsigned = false;
  bool _is_struct = false;
  bool _is_bool = false;
  bool _resolved = false;
  size_t _n_elements = 0;
  bool _is_lvalue = false;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
