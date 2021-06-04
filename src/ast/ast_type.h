#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include <variant>
#include "src/ast/ast_base.h"
#include "src/ast/fwd.h"
#include "src/ast/ty.h"
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

/**
 * \brief Type of an ASTNode
 */
class ASTType : public ASTBase {
public:
  static ASTType *Create();
  static ASTType *Create(CompilerSession *cs, Ty t, vector<ASTType *> sub_tys = {}, bool is_lvalue = false);

public:
  static umap<str, Ty> basic_tys;
  static umap<str, Ty> qualifier_tys;

private:
  static inline umap<Ty, ASTType *> _cache{};
  // TODO: static ASTType * find_cache(Ty t, const vector<ASTType *> &sub_tys, bool is_lvalue);

public:
  ASTType();

  ASTType(const ASTType &) = default;
  ASTType(ASTType &&) = default;
  ASTType &operator=(const ASTType &) = default;
  ASTType &operator=(ASTType &&) = default;

  bool operator==(const ASTType &other);
  bool operator!=(const ASTType &other);

  virtual str to_string(bool print_prefix = true);

public:
  Ty _tyty = Ty::INVALID; // FIXME: fix this goddamn name
  // use variant to prevent non-trivial destructor problem
  std::variant<str, uint64_t, float, double> _default_value;
  str _type_name = "";
  llvm::Type *_llvm_type = nullptr;
  size_t _size_bits = 0;
  size_t _align_bits = 0;
  unsigned _dwarf_encoding = 0;
  bool _is_ptr = false;
  bool _is_float = false;
  bool _is_array = false;
  size_t _array_size = 0;
  bool _is_int = false;
  bool _is_unsigned = false;
  bool _is_struct = false;
  bool _is_bool = false;
  bool _is_enum = false;
  bool _resolved = false;
  bool _is_lvalue = false;
  bool _is_forward_decl = false;

public:
  vector<ASTType *> _sub_types;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
