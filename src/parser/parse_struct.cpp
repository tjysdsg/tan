#include "src/parser/parser_impl.h"
#include "src/ast/ast_base.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"
#include "src/common.h"
#include "base.h"

using namespace tanlang;

size_t ParserImpl::parse_struct_decl(ASTBase *_p) {
  StructDecl *p = ast_must_cast<StructDecl>(_p);

  ++p->_end_index; /// skip "struct"

  /// struct typename
  auto _id = peek(p->_end_index);
  if (_id->get_node_type() != ASTNodeType::ID) {
    error(p->_end_index, "Expecting a typename");
  }
  p->_end_index = parse_node(_id);
  auto id = ast_must_cast<Identifier>(_id);
  p->set_name(id->get_name());

  /// struct body
  if (at(p->_end_index)->value == "{") {
    auto _comp_stmt = next_expression(p->_end_index, PREC_LOWEST);
    if (!_comp_stmt || _comp_stmt->get_node_type() != ASTNodeType::STATEMENT) {
      error(p->_end_index, "struct definition requires a valid body");
    }
    auto comp_stmt = ast_must_cast<CompoundStmt>(_comp_stmt);

    /// copy member declarations
    auto children = comp_stmt->get_children();
    vector<Expr *> member_decls{};
    for (const auto &c : children) {
      if (!is_ast_type_in(c->get_node_type(), {ASTNodeType::VAR_DECL, ASTNodeType::ASSIGN, ASTNodeType::FUNC_DECL})) {
        error(c->_end_index, "Invalid struct member");
      }
      member_decls.push_back(ast_must_cast<Expr>(c));
    }
    p->set_member_decls(member_decls);
  } else {
    p->set_is_forward_decl(true);
  }

  return p->_end_index;
}
