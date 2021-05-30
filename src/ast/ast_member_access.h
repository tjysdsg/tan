#ifndef __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#define __TAN_SRC_AST_AST_MEMBER_ACCESS_H__
#include "src/ast/ast_node.h"

namespace tanlang {

enum class MemberAccessType {
  MemberAccessInvalid = 0,
  MemberAccessBracket,
  MemberAccessMemberVariable,
  MemberAccessMemberFunction,
  MemberAccessDeref,
  MemberAccessEnumValue,
};

class ASTMemberAccess;
using ASTMemberAccessPtr = std::shared_ptr<ASTMemberAccess>;

class ASTMemberAccess final : public ASTNode {
public:
  ASTMemberAccess() = delete;
  ASTMemberAccess(ASTNodeType op, int lbp) : ASTNode(op, lbp) {}

public:
  MemberAccessType _access_type = MemberAccessType::MemberAccessInvalid;
  size_t _access_idx = (size_t) -1; /// struct member variable index
};

} // namespace tanlang

#endif /*__TAN_SRC_AST_AST_MEMBER_ACCESS_H__*/
