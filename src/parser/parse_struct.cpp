#include "base.h"
#include "src/parser/parser_impl.h"
#include "src/ast/ast_base.h"
#include "src/ast/expr.h"
#include "src/ast/stmt.h"
#include "src/ast/decl.h"

using namespace tanlang;

size_t ParserImpl::parse_struct_decl(const ASTBasePtr &_p) {
  ptr<StructDecl> p = ast_must_cast<StructDecl>(_p);

  ++p->_end_index; /// skip "struct"

  /// struct typename
  auto _id = peek(p->_end_index);
  if (_id->get_node_type() != ASTNodeType::ID) {
    error(p->_end_index, "Expecting a typename");
  }
  auto id = ast_must_cast<Identifier>(_id);
  p->set_name(id->get_name());

  /// struct body
  if (at(p->_end_index)->value == "{") {
    auto _comp_stmt = next_expression(p->_end_index, PREC_LOWEST);
    if (!_comp_stmt || _comp_stmt->get_node_type() == ASTNodeType::STATEMENT) {
      error(_comp_stmt->_end_index, "Invalid struct body");
    }
    auto comp_stmt = ast_must_cast<Stmt>(_comp_stmt);

    /// copy member declarations
    auto children = comp_stmt->get_children();
    vector<DeclPtr> member_decls{};
    for (const auto& c : children) {
      member_decls.push_back(expect_decl(c));
    }
    p->set_member_decls(member_decls);
  } else {
    p->set_is_forward_decl(true);
  }

  return p->_end_index;
}
