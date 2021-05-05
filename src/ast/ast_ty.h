#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include <variant>
#include "src/ast/source_traceable.h"
#include "base.h"

#define TY_GET_BASE(t) ((Ty)((uint64_t)t & TY_BASE_MASK))
#define TY_GET_QUALIFIER(t) ((Ty)((uint64_t)t & TY_QUALIFIER_MASK))
#define TY_IS(t1, t2) ((bool)((uint64_t)(t1) & (uint64_t)(t2)))
#define TY_OR(a, b) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b))
#define TY_OR3(a, b, c) static_cast<Ty>((uint64_t) (a) | (uint64_t) (b) | (uint64_t) (c))

namespace llvm {
class Type;
}

namespace tanlang {

#undef VOID
#undef CONST

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

class ASTTy;
using ASTTyPtr = ptr<ASTTy>;

/**
 * \brief Type of an ASTNode
 *
 * \details Globally unique (also for pointers), so you can compare two types by comparing two pointers
 * TODO: make it also immutable once created
 */
class ASTTy : public SourceTraceable, public enable_ptr_from_this<ASTTy> {
public:
  static umap<str, Ty> basic_tys;
  static umap<str, Ty> qualifier_tys;

private:
  static inline umap<Ty, ASTTyPtr> _cache{};
  static ASTTyPtr find_cache(Ty t, const vector<ASTTyPtr> &sub_tys, bool is_lvalue);

public:
  ASTTy() {}

  ASTTy(const ASTTy &) = default;
  ASTTy(ASTTy &&) = default;
  ASTTy &operator=(const ASTTy &) = default;
  ASTTy &operator=(ASTTy &&) = default;

  bool operator==(const ASTTy &other);
  bool operator!=(const ASTTy &other);

  virtual str to_string(bool print_prefix = true);

public:
  Ty _tyty = Ty::INVALID;

  vector<ASTTyPtr> _children{};

  // avoid name collision with _ty
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
  size_t _dominant_idx = 0;
  umap<str, size_t> _member_indices{};
  vector<str> _member_names{};
  bool _is_forward_decl = false;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
