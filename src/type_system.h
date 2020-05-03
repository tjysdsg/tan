#ifndef __TAN_SRC_AST_TYPE_SYSTEM_H__
#define __TAN_SRC_AST_TYPE_SYSTEM_H__
#include "src/ast/ast_node.h"

namespace llvm {
class DISubroutineType;
}

namespace tanlang {

class ASTTy;
using ASTTyPtr = std::shared_ptr<ASTTy>;

class TypeSystem {
public:
  static constexpr std::array LiteralTypes = {ASTType::NUM_LITERAL, ASTType::STRING_LITERAL, ASTType::ARRAY_LITERAL,};

  /**
   * \brief Find out which type should a value be implicitly cast to.
   * \details Return 0 if t1, 1 if t2, and -1 if can't. If both ok, 0 is returned.
   * */
  static int CanImplicitCast(ASTTyPtr t1, ASTTyPtr t2);

  /**
   * \brief Convert a value to from orig type to dest type.
   * \details Returns nullptr if failed to convert.
   * \param dest Destination type.
   * \param val Value to convert.
   * \param is_lvalue True if orig_val is an lvalue.
   * \param is_signed True if orig_val is a signed integer, or dest type is a signed integer.
   *                  Only used if converting from int to float, otherwise ignored.
   * \return Converted value if convertible, otherwise `nullptr`.
   * */
  static llvm::Value *ConvertTo(CompilerSession *,
      llvm::Type *dest,
      llvm::Value *val,
      bool is_lvalue,
      bool is_signed = false);
};

llvm::DISubroutineType *create_function_type(CompilerSession *,
    llvm::Metadata *ret,
    std::vector<llvm::Metadata *> args);

} // namespace

#endif /* __TAN_SRC_AST_TYPE_SYSTEM_H__ */
