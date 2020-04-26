#ifndef __TAN_SRC_AST_AST_STRING_LITERAL_H__
#define __TAN_SRC_AST_AST_STRING_LITERAL_H__
#include "src/ast/ast_literal.h"

namespace tanlang {

struct Token;

class ASTStringLiteral final : public ASTLiteral {
public:
  ASTStringLiteral() = delete;
  ASTStringLiteral(Token *token, size_t token_index);
  ASTStringLiteral(std::string str, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::shared_ptr<ASTTy> get_ty() const override;
  std::string get_string() const;

protected:
  size_t nud(Parser *parser) override;

private:
  std::string _svalue;
  llvm::Value *_llvm_value = nullptr;
  llvm::Type *_llvm_type = nullptr;
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STRING_LITERAL_H__ */
