#ifndef __TAN_INCLUDE_INTRINSIC_H__
#define __TAN_INCLUDE_INTRINSIC_H__
#include <unordered_map>
#include <string>
#include <memory>
#include "src/ast/astnode.h"

namespace llvm {
class Value;
}

namespace tanlang {

class CompilerSession;

enum class IntrinsicType {
  ASSERT, ABORT, ASM, SWAP, MEMSET, MEMCPY, RANGE, CAST, COMP_PRINT, /// compile-time print
  FILENAME, /// name of a source file
  LINENO, /// line number of certain code
  DEFINE, /// macro definition
  LIKELY, UNLIKELY, NOOP,

  EXPECT,

  // type support

  SIZE_OF, OFFSET_OF, ALIGN_OF, ISA,

  // numeric support

  MIN_OF, MAX_OF, IS_SIGNED,

};

class Intrinsic : public ASTNode {
public:
  static std::unordered_map<std::string, IntrinsicType> intrinsics;
  static void InitCodegen(CompilerSession *compiler_session);
  static llvm::Function *GetIntrinsic(IntrinsicType type, CompilerSession *compiler_session);

public:
  Intrinsic() = delete;
  Intrinsic(Token *token, size_t token_index);
  virtual ~Intrinsic() = default;
  [[nodiscard]] virtual size_t parse(const std::shared_ptr<ASTNode> &left, Parser *parser);
  [[nodiscard]] virtual size_t parse(Parser *parser);
  std::string to_string(bool print_prefix = true) const override;
  virtual llvm::Value *codegen(CompilerSession *compiler_session);

  llvm::Value *get_llvm_value(CompilerSession *) const override { return _llvm_value; }

  /// intrinsics are always rvalue
  bool is_lvalue() const override { return false; }

protected:
  void determine_type();

protected:
  IntrinsicType _intrinsic_type;
  llvm::Value *_llvm_value = nullptr;
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_INTRINSIC_H__ */
