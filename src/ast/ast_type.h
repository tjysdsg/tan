#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include <variant>
#include "src/ast/ast_base.h"
#include "src/ast/fwd.h"
#include "src/ast/ty.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"
#include "base.h"

#define TY_GET_BASE(t) ((Ty)((uint64_t)t & TY_BASE_MASK))
#define TY_GET_QUALIFIER(t) ((Ty)((uint64_t)t & TY_QUALIFIER_MASK))
#define TY_IS(t1, t2) ((bool)((uint64_t)(t1) & (uint64_t)(t2)))
#define TY_OR(a, b) static_cast<Ty>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b))
#define TY_OR3(a, b, c) static_cast<Ty>(static_cast<uint64_t>(a) | static_cast<uint64_t>(b) | static_cast<uint64_t>(c))

namespace llvm {
class Type;
}

namespace tanlang {

/**
 * \brief Type of an ASTNode
 */
class ASTType : public ASTBase {
public:
  static ASTType *Create(CompilerSession *cs);
  static ASTType *CreateAndResolve(CompilerSession *cs, Ty t, vector<ASTType *> sub_tys = {}, bool is_lvalue = false);

public:
  static umap<str, Ty> basic_tys;
  static umap<str, Ty> qualifier_tys;

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
  using default_value_t = std::variant<str, uint64_t, float, double>;

  Ty get_ty() const;
  void set_ty(Ty tyty);

  Constructor *get_constructor() const;
  void set_constructor(Constructor *constructor);
  const str &get_type_name() const;
  void set_type_name(const str &type_name);
  llvm::Type *get_llvm_type() const;
  void set_llvm_type(llvm::Type *llvm_type);
  size_t get_size_bits() const;
  void set_size_bits(size_t size_bits);
  size_t get_align_bits() const;
  void set_align_bits(size_t align_bits);
  unsigned int get_dwarf_encoding() const;
  void set_dwarf_encoding(unsigned int dwarf_encoding);
  bool is_ptr() const;
  void set_is_ptr(bool is_ptr);
  bool is_float() const;
  void set_is_float(bool is_float);
  bool is_array() const;
  void set_is_array(bool is_array);
  size_t get_array_size() const;
  void set_array_size(size_t array_size);
  bool is_int() const;
  void set_is_int(bool is_int);
  bool is_unsigned() const;
  void set_is_unsigned(bool is_unsigned);
  bool is_struct() const;
  void set_is_struct(bool is_struct);
  bool is_bool() const;
  void set_is_bool(bool is_bool);
  bool is_enum() const;
  void set_is_enum(bool is_enum);
  bool is_resolved() const;
  void set_resolved(bool resolved);
  bool is_forward_decl() const;
  void set_is_forward_decl(bool is_forward_decl);
  vector<ASTType *> &get_sub_types();
  void set_sub_types(const vector<ASTType *> &sub_types);
  ASTType *get_canonical_type() const;

  /**
   * \brief Unlike other attributes, is_lvalue() and set_is_lvalue() do not look through/modify the canonical type
   * The reason is that, for example, multiple variables all have a type reference to the same struct type, but some of
   * them are lvalues while others aren't, then calling is_lvalue()/set_is_lvalue() should only operate on the
   * type references themselves.
   */
  bool is_lvalue() const;

  /**
   * \brief Unlike other attributes, is_lvalue() and set_is_lvalue() do not look through/modify the canonical type.
   * The reason is that, for example, multiple variables all have a type reference to the same struct type, but some of
   * them are lvalues while others aren't, then calling is_lvalue()/set_is_lvalue() should only operate on the
   * type references themselves.
   */
  void set_is_lvalue(bool is_lvalue);

private:
  ASTType *must_get_canonical_type() const;
  void no_modifications_on_type_reference() const;

private:
  Ty _tyty = Ty::INVALID; // FIXME: fix this goddamn name
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
  vector<ASTType *> _sub_types;
  Constructor *_constructor = nullptr;
  CompilerSession *_cs = nullptr;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
