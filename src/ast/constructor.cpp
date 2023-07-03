#include "ast/constructor.h"
#include "ast/expr.h"
#include "ast/type.h"

using namespace tanlang;

BasicConstructor *BasicConstructor::Create(CompTimeExpr *val) {
  auto *ret = new BasicConstructor();
  TAN_ASSERT(val->get_type());
  ret->_value = val;
  return ret;
}

CompTimeExpr *BasicConstructor::get_value() const { return _value; }

StructConstructor::StructConstructor(Type *struct_type)
    : Constructor(ConstructorType::STRUCT), _struct_type(struct_type) {}

StructConstructor *StructConstructor::Create(Type *struct_type) { return new StructConstructor(struct_type); }

StructConstructor *StructConstructor::Create(Type *struct_type, vector<Constructor *> member_ctrs) {
  auto *ret = new StructConstructor(struct_type);
  ret->_member_constructors = std::move(member_ctrs);
  return ret;
}

vector<Constructor *> &StructConstructor::get_member_constructors() { return _member_constructors; }

void StructConstructor::set_member_constructors(const vector<Constructor *> &member_constructors) {
  _member_constructors = member_constructors;
}

Type *StructConstructor::get_struct_type() const { return _struct_type; }
