#ifndef __TAN_SRC_AST_TYPE_SYSTEM_H__
#define __TAN_SRC_AST_TYPE_SYSTEM_H__
#include "base.h"
#include "src/ast/ast_type.h"
#include <array>

namespace llvm {
class DISubroutineType;
class Type;
class Metadata;
class Value;
}

namespace tanlang {

AST_FWD_DECL(ASTTy);

class TypeSystem {
public:
  static constexpr std::array LiteralTypes = {ASTType::NUM_LITERAL, ASTType::STRING_LITERAL, ASTType::ARRAY_LITERAL,};

  /**
   * \brief Find out which type should a value be implicitly cast to.
   * \details Return 0 if t1, 1 if t2, and -1 if can't. If both ok, 0 is returned.
   * */
  static int CanImplicitCast(CompilerSession *cs, ASTTyPtr t1, ASTTyPtr t2);

  /**
   * \brief Set the fields of an ASTTy according to the type and target machine
   */
  static void ResolveTy(CompilerSession *cs, ASTTyPtr p);

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

  static llvm::Type *ToLLVMType(CompilerSession *cs, const ASTTyPtr &p);

  static llvm::Metadata *ToLLVMMeta(CompilerSession *cs, const ASTTyPtr &p);

  static llvm::DISubroutineType *CreateFunctionDIType(CompilerSession *, llvm::Metadata *ret, vector<llvm::Metadata *> args);
};

} // namespace

#endif /* __TAN_SRC_AST_TYPE_SYSTEM_H__ */
