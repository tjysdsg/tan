#ifndef __TAN_INCLUDE_INTRINSIC_H__
#define __TAN_INCLUDE_INTRINSIC_H__
#include <unordered_map>
#include <string>
#include <memory>
#include "src/ast/astnode.h"
#include "stack_trace.h"

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

  // others

  GET_DECL, STACK_TRACE,
};

class Intrinsic : public ASTNode {
public:
  static llvm::Value *stack_trace;
  static llvm::Type *stack_trace_t;
public:
  static std::unordered_map<std::string, IntrinsicType> intrinsics;
  static void InitCodegen(CompilerSession *compiler_session);
  static void RuntimeInit(CompilerSession *compiler_session);
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
  bool is_lvalue() const override { return _is_lvalue; }

protected:
  void determine_type();
  void parse_get_decl();

protected:
  IntrinsicType _intrinsic_type;
  llvm::Value *_llvm_value = nullptr;
  std::string _str_data = "";
  bool _is_lvalue = false;
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_INTRINSIC_H__ */
