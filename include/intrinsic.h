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
  ABORT, ASM, SWAP, MEMSET, MEMCPY, RANGE, COMP_PRINT, /// compile-time print
  FILENAME, /// name of a source file
  LINENO, /// line number of certain code
  DEFINE, /// macro definition
  LIKELY, UNLIKELY, NOOP, EXPECT,

  // type support
  SIZE_OF, OFFSET_OF, ALIGN_OF, ISA,

  // numeric support
  MIN_OF, MAX_OF, IS_SIGNED,

  // others
  GET_DECL, STACK_TRACE,
};

class Intrinsic : public ASTNode {
public:
  static bool assert_initialized;

public:
  static std::unordered_map<std::string, IntrinsicType> intrinsics;
  static void InitCodegen(CompilerSession *);
  static void RuntimeInit(CompilerSession *);
  static llvm::Function *GetIntrinsic(IntrinsicType type, CompilerSession *);

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
  IntrinsicType _intrinsic_type;
  std::string _str_data = "";
  bool _is_lvalue = false;
  bool _is_typed = true;
};

} // namespace tanlang

#endif /* __TAN_INCLUDE_INTRINSIC_H__ */
