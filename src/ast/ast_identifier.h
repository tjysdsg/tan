#ifndef TAN_SRC_AST_AST_IDENTIFIER_H_
#define TAN_SRC_AST_AST_IDENTIFIER_H_
#include "src/ast/astnode.h"

namespace tanlang {

class ASTIdentifier final : public ASTNode {
public:
  ASTIdentifier() = delete;
  ASTIdentifier(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  bool is_named() const override;
  std::string get_name() const override;
  std::string to_string(bool print_prefix = true) const override;
  llvm::Value *get_llvm_value(CompilerSession *) const override;
  bool is_lvalue() const override;
  bool is_typed() const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;
  std::shared_ptr<ASTTy> get_ty() const override;

protected:
  size_t nud(Parser *parser) override;
  ASTNodePtr get_referred() const;

private:
  std::string _name{};
  llvm::Value *_llvm_value = nullptr;
  mutable ASTNodePtr _referred = nullptr;
};

} // namespace tanlang

#endif //TAN_SRC_AST_AST_IDENTIFIER_H_
