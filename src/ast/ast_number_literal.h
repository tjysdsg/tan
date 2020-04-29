#ifndef __TAN_SRC_AST_AST_NUMBER_LITERAL_H__
#define __TAN_SRC_AST_AST_NUMBER_LITERAL_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

struct Token;

class ASTNumberLiteral final : public ASTLiteral {
public:
  friend class ASTTy;

  friend class ASTMemberAccess;

  friend class Intrinsic;

public:
  explicit ASTNumberLiteral(int value, size_t token_index, bool is_unsigned = false);
  explicit ASTNumberLiteral(size_t value, size_t token_index, bool is_unsigned = true);
  explicit ASTNumberLiteral(float value, size_t token_index);
  ASTNumberLiteral(const std::string &str, bool is_float, Token *token, size_t token_index);
  [[nodiscard]] bool is_float() const;
  Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *compiler_session) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::string to_string(bool print_prefix = true) const override;
  std::shared_ptr<ASTTy> get_ty() const override;

protected:
  size_t nud(Parser *parser) override;

private:
  bool _is_float = false;
  bool _is_unsigned = false;
  union {
    int _ivalue;
    float _fvalue;
  };
  llvm::Value *_llvm_value = nullptr;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_NUMBER_LITERAL_H__ */
