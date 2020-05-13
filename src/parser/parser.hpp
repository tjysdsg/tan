#include "src/ast/ast_ty.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_statement.h"
#include "src/parser/token_check.h"
#include "src/ast/ast_identifier.h"

namespace tanlang {

#define PARSE_TEMPLATE_SPECIALIZATION(ast, t)                                          \
  template <>                                                                          \
  ASTNodePtr Parser::parse<ASTType::t>(size_t & index, bool strict) {                  \
    auto *token = this->at(index);                                                     \
    ASTNodePtr node = std::make_shared<ast>(token, index);                             \
    if (strict) {                                                                      \
      index = node->parse(this, _cs);                                                  \
    } else {                                                                           \
      try {                                                                            \
        index = node->parse(this, _cs);                                                \
      } catch (const std::runtime_error &e) {                                          \
        return nullptr;                                                                \
      }                                                                                \
    }                                                                                  \
    return node;                                                                       \
  }

PARSE_TEMPLATE_SPECIALIZATION(ASTStatement, STATEMENT)
PARSE_TEMPLATE_SPECIALIZATION(ASTIdentifier, ID)
PARSE_TEMPLATE_SPECIALIZATION(ASTTy, TY)

#undef PARSE_TEMPLATE_SPECIALIZATION

} // namespace tanlang
