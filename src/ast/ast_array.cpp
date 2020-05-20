namespace tanlang {

Value *ASTArrayLiteral::_codegen(CompilerSession *cs) {
  _llvm_value = _ty->get_llvm_value(cs);
  return _llvm_value;
}

str ASTArrayLiteral::to_string(bool print_prefix) {
  str ret;
  if (print_prefix) { ret = ASTLiteral::to_string(true) + " "; }
  ret += "[";
  size_t i = 0;
  size_t n = _children.size();
  for (auto c : _children) {
    ret += c->to_string(false);
    if (i < n - 1) { ret += ", "; }
    ++i;
  }
  return ret + "]";
}

} // namespace tanlang
