#include "src/analysis/ast_helper.h"
#include "src/ast/ast_base.h"
#include "src/ast/ast_type.h"
#include "compiler_session.h"

using namespace tanlang;

// TODO: remove ASTHelper

ASTHelper::ASTHelper(CompilerSession *cs) : _cs(cs) {}

ASTType *ASTHelper::get_ptr_to(ASTType *p) const {
  return ASTType::CreateAndResolve(_cs, p->get_loc(), Ty::POINTER, {p}, false);
}

ASTType *ASTHelper::get_contained_ty(ASTType *p) const {
  if (p->get_ty() == Ty::STRING) {
    return ASTType::CreateAndResolve(_cs, p->get_loc(), Ty::CHAR, {}, false);
  } else if (p->is_ptr()) {
    TAN_ASSERT(p->get_sub_types().size());
    auto ret = p->get_sub_types()[0];
    TAN_ASSERT(ret);
    return ret;
  } else {
    return nullptr;
  }
}
