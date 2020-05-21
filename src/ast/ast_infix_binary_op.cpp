llvm::Metadata *ASTInfixBinaryOp::to_llvm_meta(CompilerSession *cs) {
  TAN_ASSERT(_children.size() > _dominant_idx);
  return _children[_dominant_idx]->to_llvm_meta(cs);
}

