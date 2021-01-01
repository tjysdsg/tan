#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include "src/ast/ast_node.h"
#include <variant>

#define TY_GET_BASE(t) ((Ty)((uint64_t)t & TY_BASE_MASK))
#define TY_GET_QUALIFIER(t) ((Ty)((uint64_t)t & TY_QUALIFIER_MASK))
#define TY_IS(t1, t2) ((bool)((uint64_t)(t1) & (uint64_t)(t2)))
#define TY_OR(a, b) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b))
#define TY_OR3(a, b, c) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b) | (uint64_t) (c))

namespace tanlang {

#undef VOID
#undef CONST

enum class Ty : uint64_t {
  INVALID = 0,
  /// basic types 1->12 bits
  #define TY_BASE_MASK 0xfffu
  VOID = 1u, INT = 2u, FLOAT = 3u, DOUBLE = 4u, BOOL = 5u,
  POINTER = 6u,
  STRING = 7u,
  CHAR = 8u,
  FUNC_PTR = 9u, // TODO: function ptr
  STRUCT = 10u,
  ARRAY = 11u,
  ENUM = 12u,

  /// qualifiers 13->32 bits
  #define TY_QUALIFIER_MASK 0xffffff000u

  UNSIGNED = 1u << 13u,
  CONST = 1u << 14u,
  BIT8 = 1u << 15u,
  BIT16 = 1u << 16u,
  BIT32 = 1u << 17u,
  BIT64 = 1u << 18u,
};

// TODO: make ASTTy immutable, like llvm::Type
class ASTTy : public ASTNode, public enable_ptr_from_this<ASTTy> {
private:
  static inline umap<Ty, ASTTyPtr> _cache{};
  static ASTTyPtr find_cache(Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue);

public:
  ASTTy();
  ASTTy(const ASTTy &) = default;
  ASTTy(ASTTy &&) = default;
  ASTTy &operator=(const ASTTy &);
  ASTTy &operator=(ASTTy &&);
  bool operator==(const ASTTy &other);
  bool operator!=(const ASTTy &other);
  str to_string(bool print_prefix = true) override;

public:
  // avoid name collision with _ty
  Ty _tyty = Ty::INVALID;
  // use variant to prevent non-trivial destructor problem
  std::variant<str, uint64_t, float, double> _default_value;
  str _type_name = "";
  llvm::Type *_llvm_type = nullptr;
  size_t _size_bits = 0;
  size_t _align_bits = 0;
  unsigned _dwarf_encoding = 0;
  bool _is_ptr = false;
  bool _is_float = false;
  // TODO: size bits for float
  bool _is_array = false;
  size_t _array_size = 0;
  bool _is_int = false;
  bool _is_unsigned = false;
  bool _is_struct = false;
  bool _is_bool = false;
  bool _is_enum = false;
  bool _resolved = false;
  bool _is_lvalue = false;
  umap<str, size_t> _member_indices{};
  vector<str> _member_names{};
  bool _is_forward_decl = false;
};

umap<str, Ty> basic_tys =
    {{"int", TY_OR(Ty::INT, Ty::BIT32)}, {"float", Ty::FLOAT}, {"double", Ty::DOUBLE}, {"i8", TY_OR(Ty::INT, Ty::BIT8)},
        {"u8", TY_OR3(Ty::INT, Ty::BIT8, Ty::UNSIGNED)}, {"i16", TY_OR(Ty::INT, Ty::BIT16)},
        {"u16", TY_OR3(Ty::INT, Ty::BIT16, Ty::UNSIGNED)}, {"i32", TY_OR(Ty::INT, Ty::BIT32)},
        {"u32", TY_OR3(Ty::INT, Ty::BIT32, Ty::UNSIGNED)}, {"i64", TY_OR(Ty::INT, Ty::BIT64)},
        {"u64", TY_OR3(Ty::INT, Ty::BIT64, Ty::UNSIGNED)}, {"void", Ty::VOID}, {"str", Ty::STRING}, {"char", Ty::CHAR},
        {"bool", Ty::BOOL},};

umap<str, Ty> qualifier_tys = {{"const", Ty::CONST}, {"unsigned", Ty::UNSIGNED}, {"*", Ty::POINTER},};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
