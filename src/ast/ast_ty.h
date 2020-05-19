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

class Parser;
class ASTTy;
using ASTTyPtr = std::shared_ptr<ASTTy>;

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

class ASTTy : public ASTNode, public std::enable_shared_from_this<ASTTy> {
public:
  static std::shared_ptr<ASTTy> Create(Ty t, vector<ASTNodePtr> sub_tys = {}, bool is_lvalue = false);

private:
  static inline umap<Ty, ASTTyPtr> _cache{};
  static ASTTyPtr find_cache(Ty t, vector<ASTNodePtr> sub_tys, bool is_lvalue);

public:
  ASTTy() = delete;
  ASTTy(Token *token, size_t token_index);
  ASTTy(const ASTTy &) = default;
  ASTTy(ASTTy &&) = default;
  ASTTy &operator=(const ASTTy &);
  ASTTy &operator=(ASTTy &&);

public:
  str get_type_name();
  llvm::Metadata *to_llvm_meta(CompilerSession *) override;
  str to_string(bool print_prefix = true) override;
  llvm::Type *to_llvm_type(CompilerSession *) override;
  llvm::Value *get_llvm_value(CompilerSession *) override;

public:
  bool is_lvalue() override;
  bool is_typed() override;
  ASTTyPtr get_contained_ty();
  ASTTyPtr get_ptr_to();
  bool operator==(const ASTTy &other);
  bool operator!=(const ASTTy &other);
  void set_is_lvalue(bool is_lvalue);
  size_t get_size_bits();
  bool is_ptr();
  bool is_float();
  bool is_floating();
  bool is_double();
  bool is_int();
  bool is_bool();
  bool is_enum();
  bool is_unsigned();
  bool is_struct();
  bool is_array();
  size_t get_n_elements();
  size_t get_member_index(str name);
  ASTNodePtr get_member(size_t i);

public:
  // avoid name collisions
  Ty _tyty = Ty::INVALID;
  // use variant to prevent non-trivial destructor problem
  std::variant<str, uint64_t, float, double> _default_value;

protected:
  llvm::Value *_codegen(CompilerSession *) override { return nullptr; }
  size_t nud_array();
  size_t nud_struct();
  size_t nud() override;
  void resolve();
  str _type_name = "";
  llvm::Type *_llvm_type = nullptr;
  size_t _size_bits = 0;
  size_t _align_bits = 0;
  unsigned _dwarf_encoding = 0;
  bool _is_ptr = false;
  bool _is_float = false;
  bool _is_array = false;
  bool _is_double = false;
  bool _is_int = false;
  bool _is_unsigned = false;
  bool _is_struct = false;
  bool _is_bool = false;
  bool _is_enum = false;
  bool _resolved = false;
  size_t _n_elements = 0;
  bool _is_lvalue = false;
  umap<str, size_t> _member_indices{};
  vector<str> _member_names{};
  bool _is_forward_decl = true;
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_TY_H_ */
