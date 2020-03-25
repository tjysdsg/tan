#ifndef __TAN_SRC_AST_AST_STRUCT_H__
#define __TAN_SRC_AST_AST_STRUCT_H__
#include "src/ast/astnode.h"
#include <unordered_map>

namespace tanlang {
class ASTStruct : public ASTNode, public std::enable_shared_from_this<ASTStruct>, public Named, public Typed {
public:
  ASTStruct() = delete;
  explicit ASTStruct(Token *token);
  void nud(Parser *parser) override;
  Value *codegen(CompilerSession *compiler_session) override;
  size_t get_member_index(std::string name);
  std::string get_name() const override;
  std::string get_type_name() const override;
  llvm::Type *to_llvm_type(CompilerSession *) const override;

private:
  llvm::Type *_llvm_type = nullptr;
  std::unordered_map<std::string, size_t> _member_indices{};
};
} // namespace tanlang

#endif /* __TAN_SRC_AST_AST_STRUCT_H__ */
