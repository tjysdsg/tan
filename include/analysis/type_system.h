#ifndef __TAN_SRC_AST_TYPE_SYSTEM_H__
#define __TAN_SRC_AST_TYPE_SYSTEM_H__
#include "base.h"
#include "ast/ast_node_type.h"
#include "ast/fwd.h"
#include <array>

namespace llvm {
class DISubroutineType;
class Type;
class Metadata;
class Value;
} // namespace llvm

namespace tanlang {

class CompilerSession;

class TypeSystem {
public:
  static constexpr std::array LiteralTypes = {ASTNodeType::INTEGER_LITERAL, ASTNodeType::FLOAT_LITERAL,
                                              ASTNodeType::STRING_LITERAL, ASTNodeType::ARRAY_LITERAL};

  /**
   * \brief Convert a value to from orig type to dest type.
   * \details Returns nullptr if failed to convert.
   * \param dest Destination type.
   * \param expr Original expression.
   * \return Converted value if convertible, otherwise `nullptr`. Note that the returned value is always rvalue. To
   * get an lvalue, create a temporary variable and store the value to it.
   * */
  static llvm::Value *ConvertTo(CompilerSession *cs, Expr *expr, Type *dest);

  /**
   * \brief Check whether it's legal to implicitly convert from type `from` to type `to`
   *        See TYPE_CASTING.md for specifications.
   * \param from Source type.
   * \param to Destination type.
   */
  static bool CanImplicitlyConvert(Type *from, Type *to);

  /**
   * \brief Find out which one of the two input types of a binary operation should operands promote to.
   *        See TYPE_CASTING.md for specifications.
   * \return Guaranteed to be one of `t1` and `t2`, or nullptr if cannot find a legal promotion.
   */
  static Type *ImplicitTypePromote(Type *t1, Type *t2);

  /**
   * \brief Create a load instruction if the type is lvalue. Otherwise return the original value.
   */
  static llvm::Value *LoadIfLValue(CompilerSession *cs, Expr *expr);

  static llvm::Type *ToLLVMType(CompilerSession *cs, Type *p);

  static llvm::Metadata *ToLLVMMeta(CompilerSession *cs, Type *p);

  static llvm::DISubroutineType *CreateFunctionDIType(CompilerSession *cs, llvm::Metadata *ret,
                                                      vector<llvm::Metadata *> args);
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_TYPE_SYSTEM_H__ */
