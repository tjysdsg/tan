namespace tanlang {

llvm::Value *ASTProgram::_codegen(CompilerSession *cs) {
  for (const auto &e : _children) { e->codegen(cs); }
  return nullptr;
}

} // namespace tanlang
