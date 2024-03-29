#ifndef __TAN_INCLUDE_INTRINSIC_H__
#define __TAN_INCLUDE_INTRINSIC_H__
#include "expr.h"
#include "ast_named.h"
#include "stmt.h"
#include <memory>

namespace llvm {
class Value;
class Function;
} // namespace llvm

namespace tanlang {

enum class IntrinsicType {
  INVALID = 0,

  ABORT,      /// abort
  ASM,        /// inline assembly
  SWAP,       /// swap, atomic for primitive types
  MEMSET,     /// memset
  MEMCPY,     /// memcpy
  RANGE,      /// range of integers
  COMP_PRINT, /// compile-time print
  FILENAME,   /// name of a source file
  LINENO,     /// line number of certain code
  DEFINE,     /// macro definition
  LIKELY,     /// likely
  UNLIKELY,   /// unlikely
  NOOP,       /// no-op
  EXPECT,     /// expect a value to be true, used in tests

  // type support
  SIZE_OF,   /// size of a type, in bytes
  OFFSET_OF, /// offset of a member variable inside a struct, in bytes
  ALIGN_OF,  /// alignment size of a pointer, array or struct, in bytes
  ISA,       /// test if a type is equivalent to another one

  // numeric support
  MIN_OF,      /// minimum value of an integer type
  MAX_OF,      /// maximum value of an integer type
  IS_UNSIGNED, /// test if a type is unsigned

  // test related
  TEST_COMP_ERROR, /// A block of code that are expected to cause a compile error

  // others
  GET_DECL,    /// get source code of the declaration
  STACK_TRACE, /// print stack trace
};

/**
 * \brief A generic representation of Intrinsic variables/functions
 */
class Intrinsic : public Expr, public ASTNamed {
protected:
  explicit Intrinsic(TokenizedSourceFile *src);

public:
  static Intrinsic *Create(TokenizedSourceFile *src);

  /**
   * \brief A mapping from intrinsic names to IntrinsicType
   */
  static umap<str, IntrinsicType> INTRINSIC_NAME_TO_TYPES;

  /**
   * \brief Generate a list of intrinsics function prototypes/declarations, such as `@abort`
   */
  static vector<FunctionDecl *> GetIntrinsicFunctionDeclarations();

  static inline const str STACK_TRACE_FUNCTION_REAL_NAME = "__tan_runtime_stack_trace";
  static inline const str TEST_COMP_ERROR_NAME = "test_comp_error";
  static inline const str ABORT_NAME = "abort";
  static inline const str COMP_PRINT_NAME = "compprint";

public:
  IntrinsicType get_intrinsic_type() const;
  void set_intrinsic_type(IntrinsicType intrinsic_type);
  ASTBase *get_sub() const;
  void set_sub(ASTBase *sub);

  vector<ASTBase *> get_children() const override;

public:
  str terminal_token() const override;

private:
  IntrinsicType _intrinsic_type = IntrinsicType::INVALID;
  ASTBase *_sub = nullptr;
};

class TestCompError : public CompoundStmt {
public:
  explicit TestCompError(TokenizedSourceFile *src) : CompoundStmt(src) {}

  bool _caught = false;
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_INTRINSIC_H__ */
