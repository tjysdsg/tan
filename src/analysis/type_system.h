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
   * \param val Value to convert. This function automatically create a `load` instruction if orig is lvalue.
   * \param dest Destination type.
   * \param orig Original type.
   * \return Converted value if convertible, otherwise `nullptr`. Note that the returned value is always rvalue. To
   * get an lvalue, create a temporary variable and store the value to it.
   * */
  static llvm::Value *ConvertTo(CompilerSession *, llvm::Value *val, ASTTyPtr orig, ASTTyPtr dest);
};

llvm::DISubroutineType *create_function_type(CompilerSession *, llvm::Metadata *ret, vector<llvm::Metadata *> args);

} // namespace

#endif /* __TAN_SRC_AST_TYPE_SYSTEM_H__ */
