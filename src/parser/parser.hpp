#include "src/ast/ast_array.h"
#include "src/ast/ast_member_access.h"
#include "src/ast/ast_expr.h"
#include "src/ast/ast_func.h"
#include "src/ast/ast_statement.h"
#include "src/parser/token_check.h"

namespace tanlang {

#define PARSE_TEMPLATE_SPECIALIZATION(ast, t)                                          \
  template <>                                                                          \
  std::shared_ptr<ASTNode> Parser::parse<ASTType::t>(size_t & index, bool strict) {    \
    auto *token = this->at(index);                                                     \
    std::shared_ptr<ASTNode> node =                                                    \
        std::make_shared<ast>(token, index);                                           \
    if (strict) {                                                                      \
      index = node->parse(this);                                                       \
    } else {                                                                           \
      try {                                                                            \
        index = node->parse(this);                                                     \
      } catch (const std::runtime_error &e) {                                          \
        return nullptr;                                                                \
      }                                                                                \
    }                                                                                  \
    return node;                                                                       \
  }

PARSE_TEMPLATE_SPECIALIZATION(ASTStatement, STATEMENT)

PARSE_TEMPLATE_SPECIALIZATION(ASTIdentifier, ID)

PARSE_TEMPLATE_SPECIALIZATION(ASTFunctionCall, FUNC_CALL)

PARSE_TEMPLATE_SPECIALIZATION(ASTTy, TY)

PARSE_TEMPLATE_SPECIALIZATION(ASTArrayLiteral, ARRAY_LITERAL)

#undef PARSE_TEMPLATE_SPECIALIZATION

}
