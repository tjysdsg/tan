#include "ast/default_value.h"
#include "ast/type.h"

using namespace tanlang;

Literal *DefaultValue::CreateTypeDefaultValueLiteral(SourceFile *src, Type *type) {
  Literal *ret = nullptr;

  if (type->is_pointer()) {
    ret = Literal::CreateNullPointerLiteral(src, ((PointerType *)type)->get_pointee());

  } else if (type->is_array()) {
    ArrayType *at = (ArrayType *)type;
    Type *element_type = at->get_element_type();
    size_t size = (size_t)at->array_size();

    vector<Literal *> elements(size);
    for (uint32_t i = 0; i < size; ++i) {
      elements[i] = DefaultValue::CreateTypeDefaultValueLiteral(src, element_type);
    }

    ret = Literal::CreateArrayLiteral(src, element_type, elements);

  } else if (type->is_string()) {
    ret = Literal::CreateStringLiteral(src, "");

  } else if (type->is_struct()) {
    // TODO: IMPLEMENT THIS

  } else if (type->is_function()) {
    // TODO: IMPLEMENT THIS

  } else if (type->is_float()) {
    ret = Literal::CreateFloatLiteral(src, 0, (size_t)type->get_size_bits());

  } else if (type->is_int()) {
    ret = Literal::CreateIntegerLiteral(src, 0, (size_t)type->get_size_bits(), type->is_unsigned());

  } else if (type->is_bool()) {
    ret = Literal::CreateBoolLiteral(src, false);

  } else if (type->is_char()) {
    ret = Literal::CreateCharLiteral(src, '\0');
  }

  TAN_ASSERT(ret);
  return ret;
}
