#ifndef __TAN_SRC_AST_AST_NUMBER_LITERAL_H__
#define __TAN_SRC_AST_AST_NUMBER_LITERAL_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

class ASTNumberLiteral final : public ASTLiteral {
public:
  friend class ASTTy;
  friend class ASTMemberAccess;
  friend class Intrinsic;

public:
  explicit ASTNumberLiteral(int value, size_t token_index, bool is_unsigned = false);
  explicit ASTNumberLiteral(size_t value, size_t token_index, bool is_unsigned = true);
  explicit ASTNumberLiteral(float value, size_t token_index);
  ASTNumberLiteral(const str &str, bool is_float, Token *token, size_t token_index);
  [[nodiscard]] bool is_float() const;
  llvm::Value *codegen(CompilerSession *compiler_session) override;
  str to_string(bool print_prefix = true) const override;

protected:
  size_t nud() override;

private:
  void resolve();
  bool _is_float = false;
  bool _is_unsigned = false;
  union {
    int _ivalue;
    float _fvalue;
  };
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_NUMBER_LITERAL_H__ */
