#ifndef __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#define __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#include "src/ast/ast_node.h"

namespace tanlang {

class ASTMemberAccess;
using ASTMemberAccessPtr = std::shared_ptr<ASTMemberAccess>;

class ASTMemberAccess final : public ASTNode {
public:
  static ASTMemberAccessPtr CreatePointerDeref(ASTNodePtr ptr);

public:
  ASTMemberAccess() = delete;
  ASTMemberAccess(Token *token, size_t token_index);
  bool is_lvalue() override;
  bool is_typed() override;

protected:
  llvm::Value *_codegen(CompilerSession *) override;
  size_t led(const ASTNodePtr &left) override;

private:
  void resolve_ptr_deref(ASTNodePtr left);

  enum MemberAccessType {
    MemberAccessInvalid = 0,
    MemberAccessBracket,
    MemberAccessMemberVariable,
    MemberAccessMemberFunction,
    MemberAccessDeref,
    MemberAccessEnumValue,
  };
  MemberAccessType _access_type = MemberAccessInvalid;
  size_t _access_idx = (size_t) -1; /// struct member variable index
  uint64_t _enum_value = 0; /// enum value
};

} // namespace tanlang

#endif /*__TAN_SRC_AST_AST_MEMBER_ACCESS_H__*/
