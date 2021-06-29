#ifndef __TAN_SRC_AST_TYPE_SYSTEM_H__
#define __TAN_SRC_AST_TYPE_SYSTEM_H__
#include "base.h"
#include "src/ast/ast_node_type.h"
#include "src/ast/fwd.h"
#include <array>

namespace llvm {
class DISubroutineType;
class Type;
class Metadata;
class Value;
}

namespace tanlang {

class TypeSystem {
public:
  static constexpr std::array LiteralTypes =
      {ASTNodeType::INTEGER_LITERAL, ASTNodeType::FLOAT_LITERAL, ASTNodeType::STRING_LITERAL,
          ASTNodeType::ARRAY_LITERAL};

  /**
   * \brief Find out which type should a value be implicitly cast to.
   * \details Return 0 if t1, 1 if t2, and -1 if can't. If both ok, 0 is returned.
   * */
  static int CanImplicitCast(CompilerSession *cs, ASTType *t1, ASTType *t2);

  /**
   * \brief Set the fields of an ASTType according to the type and target machine
   */
  static void ResolveTy(CompilerSession *cs, ASTType *const &p);

  /**
   * \brief Set the default constructor of a type
   * \details Only works for basic types, not struct
   */
  static void SetDefaultConstructor(CompilerSession *cs, ASTType *const &p);

  /**
   * \brief Convert a value to from orig type to dest type.
   * \details Returns nullptr if failed to convert.
   * \param val Value to convert. This function automatically create a `load` instruction if orig is lvalue.
   * \param dest Destination type.
   * \param orig Original type.
   * \return Converted value if convertible, otherwise `nullptr`. Note that the returned value is always rvalue. To
   * get an lvalue, create a temporary variable and store the value to it.
   * */
  static llvm::Value *ConvertTo(CompilerSession *, llvm::Value *val, ASTType *orig, ASTType *dest);

  static llvm::Type *ToLLVMType(CompilerSession *cs, ASTType *p);

  static llvm::Metadata *ToLLVMMeta(CompilerSession *cs, ASTType *p);

  static llvm::DISubroutineType *CreateFunctionDIType(CompilerSession *,
      llvm::Metadata *ret,
      vector<llvm::Metadata *> args);
};

} // namespace

#endif /* __TAN_SRC_AST_TYPE_SYSTEM_H__ */
