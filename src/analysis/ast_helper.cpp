#include "src/analysis/ast_helper.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"

using namespace tanlang;

ASTHelper::ASTHelper(CompilerSession *cs) : _cs(cs) {}

ASTType *ASTHelper::get_ptr_to(ASTType *p) const {
  return ASTType::Create(_cs, Ty::POINTER, {p}, false);
}

ASTType *ASTHelper::get_contained_ty(ASTType *p) const {
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

str ASTHelper::get_source_location(SourceTraceable *p) const {
  return _cs->_filename + ":" + std::to_string(p->get_line());
}
