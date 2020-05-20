#ifndef __TAN_SRC_ANALYSIS_H__
#define __TAN_SRC_ANALYSIS_H__
#include "base.h"
#include "src/ast/ast_node.h"

namespace tanlang {

class Analyzer final {
public:
  /// \section general
  static str get_name(ASTNodePtr p);
  static size_t get_n_children(ASTNodePtr p);
  static bool is_lvalue(ASTNodePtr p);
  static bool is_lvalue(ASTTyPtr p);
  static bool is_typed(ASTNodePtr p);

  /// \section types

  static ASTTyPtr create_ty(Ty t, vector<ASTNodePtr> sub_tys = {}, bool is_lvalue = false);
  static ASTTyPtr get_ty(ASTNodePtr p);
  static str get_type_name(ASTNodePtr p);
  static ASTTyPtr get_contained_ty(ASTTyPtr p);
  static ASTTyPtr get_ptr_to(ASTTyPtr p);
  static size_t get_size_bits(ASTTyPtr p);
  static bool is_ptr(ASTTyPtr p);
  static bool is_float(ASTTyPtr p);
  static bool is_floating(ASTTyPtr p);
  static bool is_double(ASTTyPtr p);
  static bool is_int(ASTTyPtr p);
  static bool is_bool(ASTTyPtr p);
  static bool is_enum(ASTTyPtr p);
  static bool is_unsigned(ASTTyPtr p);
  static bool is_struct(ASTTyPtr p);
  static bool is_array(ASTTyPtr p);
  static size_t get_struct_member_index(ASTTyPtr p, str name);
  static ASTTyPtr get_struct_member_ty(ASTTyPtr p, size_t i);
  static void set_is_lvalue(ASTTyPtr p, bool is_lvalue);
  static void resolve(ASTTyPtr p);
};

} // namespace tanlang

#endif //__TAN_SRC_ANALYSIS_H__
