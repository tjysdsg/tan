#include "src/analysis/ast_helper.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "src/ast/ast_struct.h"
#include "compiler_session.h"

using namespace tanlang;

ASTHelper::ASTHelper(CompilerSession *cs) : _cs(cs) {}

ASTTypePtr ASTHelper::get_ptr_to(const ASTTypePtr &p) const {
  return ASTType::Create(_cs, Ty::POINTER, {p}, false);
}

ASTTypePtr ASTHelper::get_contained_ty(const ASTTypePtr &p) const {
  if (p->_tyty == Ty::STRING) {
    return ASTType::Create(_cs, Ty::CHAR, {}, false);
  } else if (p->_is_ptr) {
    TAN_ASSERT(p->_sub_types.size());
    auto ret = p->_sub_types[0];
    TAN_ASSERT(ret);
    return ret;
  } else {
    return nullptr;
  }
}

ASTTypePtr ASTHelper::get_struct_member_ty(const ASTStructPtr &p, size_t i) const {
  return p->_sub_types[i];
}

size_t ASTHelper::get_struct_member_index(const ASTStructPtr &p, const str &name) const {
  auto search = p->_member_indices.find(name);
  if (search == p->_member_indices.end()) {
    return (size_t) (-1);
  }
  return search->second;
}

str ASTHelper::get_source_location(SourceTraceablePtr p) const {
  return _cs->_filename + ":" + std::to_string(p->get_line());
}
