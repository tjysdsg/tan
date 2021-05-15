#include "src/ast/ast_node.h"
#include "src/ast/ast_ty.h"
#include "parser.h"
#include "token.h"
#include <iostream>

using namespace tanlang;

void ASTNode::set_value(str str_value) {
  _value = str_value;
}

void ASTNode::set_value(uint64_t int_value) {
  _value = int_value;
}

void ASTNode::set_value(double float_value) {
  _value = float_value;
}

uint64_t ASTNode::get_int_value() {
  return std::get<uint64_t>(_value);
}

str ASTNode::get_str_value() {
  return std::get<str>(_value);
}

double ASTNode::get_float_value() {
  return std::get<double>(_value);
}

ASTNode::ASTNode(ASTType op, int lbp) {
  set_node_type(op);
  set_lbp(lbp);
}
