#ifndef __TAN_INCLUDE_INTRINSIC_H__
#define __TAN_INCLUDE_INTRINSIC_H__
#include <unordered_map>
#include <string>
#include <memory>
#include "src/ast/ast_node.h"

namespace llvm {
class Value;
class Function;
}

namespace tanlang {

class CompilerSession;
class ASTTy;
using ASTTyPtr = std::shared_ptr<ASTTy>;

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

class Intrinsic : public ASTNode {
public:
  static inline llvm::Function *abort_function = nullptr;
  static std::unordered_map<std::string, IntrinsicType> intrinsics;
  static void InitCodegen(CompilerSession *);

public:
  Intrinsic() = delete;
  Intrinsic(Token *token, size_t token_index);
  size_t led(const ASTNodePtr &left) override;
  size_t nud() override;
  std::string to_string(bool print_prefix = true) const override;
  llvm::Value *codegen(CompilerSession *) override;
  bool is_lvalue() const override;
  bool is_typed() const override;

protected:
  void determine_type();
  void parse_get_decl();

protected:
  IntrinsicType _intrinsic_type = IntrinsicType::INVALID;
  std::string _str_data = "";
  bool _is_lvalue = false;
  bool _is_typed = true;
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_INTRINSIC_H__ */
