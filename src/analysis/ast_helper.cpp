#include "src/analysis/ast_helper.h"
#include "src/ast/ast_node.h"
#include "src/ast/parsable_ast_node.h"
#include "src/ast/ast_ty.h"
#include "src/ast/factory.h"
#include "compiler_session.h"

using namespace tanlang;

ASTHelper::ASTHelper(CompilerSession *cs) : _cs(cs) {}

ASTNodePtr ASTHelper::get_id_referred(const ASTNodePtr &p) const {
  return _cs->get(p->get_data<str>());
}

ASTTyPtr ASTHelper::get_ptr_to(const ASTTyPtr &p) const {
  return create_ty(_cs, Ty::POINTER, {p}, false);
}

ASTTyPtr ASTHelper::get_contained_ty(const ASTTyPtr &p) const {
  if (p->_tyty == Ty::STRING) {
    return create_ty(_cs, Ty::CHAR, {}, false);
  } else if (p->_is_ptr) {
    TAN_ASSERT(p->get_children_size());
    auto ret = p->get_child_at<ASTTy>(0);
    TAN_ASSERT(ret);
    return ret;
  } else {
    return nullptr;
  }
}

ASTTyPtr ASTHelper::get_struct_member_ty(const ASTTyPtr &p, size_t i) const {
  TAN_ASSERT(p->_tyty == Ty::STRUCT);
  return get_ty(p->get_child_at(i));
}

size_t ASTHelper::get_struct_member_index(const ASTTyPtr &p, const str &name) const {
  auto search = p->_member_indices.find(name);
  if (search == p->_member_indices.end()) {
    return (size_t) (-1);
  }
  return search->second;
}

str ASTHelper::get_source_location(SourceTraceablePtr p) const {
  return _cs->_filename + ":" + std::to_string(p->get_line());
}

ASTNodePtr ASTHelper::try_convert_to_ast_node(const ParsableASTNodePtr &p) const {
  ASTNodePtr ret = nullptr;
  if (p->get_node_type() != ASTType::TY) {
    ret = ast_must_cast<ASTNode>(p);
  }
  return ret;
}

ASTTyPtr ASTHelper::get_ty(const ParsableASTNodePtr &p) const {
  ASTNodePtr np = ast_cast<ASTNode>(p);
  TAN_ASSERT(np);
  return np->_ty;
}

