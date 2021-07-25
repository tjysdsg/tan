#ifndef TAN_SRC_AST_AST_TY_H_
#define TAN_SRC_AST_AST_TY_H_
#include <variant>
#include "src/ast/ast_base.h"
#include "src/ast/fwd.h"
#include "src/ast/ty.h"
#include "src/ast/ast_named.h"
#include "src/ast/typed.h"
#include "base.h"
#include <functional>

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
  static ASTType *Create(ASTContext *ctx, SourceIndex loc);
  static ASTType *CreateAndResolve(ASTContext *ctx,
      SourceIndex loc,
      Ty t,
      vector<ASTType *> sub_tys = {},
      bool is_lvalue = false,
      const std::function<void(ASTType *)> &attribute_setter = {});
  static ASTType *GetVoidType(ASTContext *ctx, SourceIndex loc);
  static ASTType *GetI32Type(ASTContext *ctx, SourceIndex loc, bool lvalue = false);
  static ASTType *GetI8Type(ASTContext *ctx, SourceIndex loc, bool lvalue = false);

public:
  static umap<str, Ty> basic_tys;
  static umap<str, Ty> qualifier_tys;

public:
  explicit ASTType(SourceIndex loc);

  bool operator==(const ASTType &other);
  bool operator!=(const ASTType &other);

  [[nodiscard]] str to_string(bool print_prefix) const override;
  [[nodiscard]] vector<ASTBase *> get_children() const override;

public:
  [[nodiscard]] Ty get_ty() const;
  void set_ty(Ty ty);
  [[nodiscard]] ASTType *get_contained_ty() const;
  [[nodiscard]] ASTType *get_ptr_to() const;

  [[nodiscard]] Constructor *get_constructor() const;
  void set_constructor(Constructor *constructor);
  [[nodiscard]] const str &get_type_name() const;
  void set_type_name(const str &type_name);
  [[nodiscard]] llvm::Type *get_llvm_type() const;
  void set_llvm_type(llvm::Type *llvm_type);
  [[nodiscard]] size_t get_size_bits() const;
  void set_size_bits(size_t size_bits);
  [[nodiscard]] size_t get_align_bits() const;
  void set_align_bits(size_t align_bits);
  [[nodiscard]] unsigned int get_dwarf_encoding() const;
  void set_dwarf_encoding(unsigned int dwarf_encoding);
  [[nodiscard]] bool is_ptr() const;
  [[nodiscard]] bool is_float() const;
  void set_is_float(bool is_float);
  [[nodiscard]] bool is_array() const;
  void set_is_array(bool is_array);
  [[nodiscard]] size_t get_array_size() const;
  void set_array_size(size_t array_size);
  [[nodiscard]] bool is_int() const;
  void set_is_int(bool is_int);
  [[nodiscard]] bool is_unsigned() const;
  void set_is_unsigned(bool is_unsigned);
  [[nodiscard]] bool is_struct() const;
  void set_is_struct(bool is_struct);
  [[nodiscard]] bool is_bool() const;
  void set_is_bool(bool is_bool);
  [[nodiscard]] bool is_enum() const;
  void set_is_enum(bool is_enum);
  [[nodiscard]] bool is_resolved() const;
  void set_resolved(bool resolved);
  [[nodiscard]] bool is_forward_decl() const;
  void set_is_forward_decl(bool is_forward_decl);
  vector<ASTType *> &get_sub_types();
  void set_sub_types(const vector<ASTType *> &sub_types);
  [[nodiscard]] ASTType *get_canonical_type() const;

  /**
   * \brief Unlike other attributes, is_lvalue() and set_is_lvalue() do not look through/modify the canonical type
   * The reason is that, for example, multiple variables all have a type reference to the same struct type, but some of
   * them are lvalues while others aren't, then calling is_lvalue()/set_is_lvalue() should only operate on the
   * type references themselves.
   */
  [[nodiscard]] bool is_lvalue() const;

  /**
   * \brief Unlike other attributes, is_lvalue() and set_is_lvalue() do not look through/modify the canonical type.
   * The reason is that, for example, multiple variables all have a type reference to the same struct type, but some of
   * them are lvalues while others aren't, then calling is_lvalue()/set_is_lvalue() should only operate on the
   * type references themselves.
   */
  void set_is_lvalue(bool is_lvalue);

private:
  [[nodiscard]] ASTType *must_get_canonical_type() const;
  void no_modifications_on_type_reference() const;

private:
  Ty _ty = Ty::INVALID;
  str _type_name;
  llvm::Type *_llvm_type = nullptr;
  size_t _size_bits = 0;
  size_t _align_bits = 0;
  unsigned _dwarf_encoding = 0;
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
  ASTContext *_ctx = nullptr;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
