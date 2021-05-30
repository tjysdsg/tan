#ifndef __TAN_INCLUDE_INTRINSIC_H__
#define __TAN_INCLUDE_INTRINSIC_H__
#include "src/ast/ast_base.h"
#include "src/ast/typed.h"
#include "src/ast/ast_named.h"
#include <memory>

namespace llvm {
class Value;
class Function;
}

namespace tanlang {

class CompilerSession;
AST_FWD_DECL(ASTNamed);

enum class IntrinsicType {
  INVALID = 0, ABORT, /// abort
  ASM, /// inline assembly
  SWAP, /// swap, atomic for primitive types
  MEMSET, /// memset
  MEMCPY, /// memcpy
  RANGE, /// range of integers
  COMP_PRINT, /// compile-time print
  FILENAME, /// name of a source file
  LINENO, /// line number of certain code
  DEFINE, /// macro definition
  LIKELY, /// likely
  UNLIKELY, /// unlikely
  NOOP, /// no-op
  EXPECT, /// expect a value to be true, used in tests

  // type support
  SIZE_OF, /// size of a type, in bytes
  OFFSET_OF, /// offset of a member variable inside a struct, in bytes
  ALIGN_OF, /// alignment size of a pointer, array or struct, in bytes
  ISA, /// test if a type is equivalent to another one

  // numeric support
  MIN_OF, /// minimum value of an integer type
  MAX_OF, /// maximum value of an integer type
  IS_UNSIGNED, /// test if a type is unsigned

  // others
  GET_DECL, STACK_TRACE,
};

/**
 * has type
 * has value
 * rvalue
 */
class Intrinsic : public ASTBase, public Typed, public ASTNamed {
public:
  static ptr<Intrinsic> Create();
  static inline llvm::Function *abort_function = nullptr;
  static umap<str, IntrinsicType> intrinsics;
  static void InitCodegen(CompilerSession *);
  static void InitAnalysis(CompilerSession *cs);

public:
  Intrinsic();
  void set_sub(ASTNamedPtr sub);
  ASTNamedPtr get_sub() const;
  IntrinsicType get_intrinsic_type() const;
  void set_intrinsic_type(IntrinsicType intrinsic_type);

private:
  IntrinsicType _intrinsic_type = IntrinsicType::INVALID;
  ASTNamedPtr _sub = nullptr;
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_INTRINSIC_H__ */
