#ifndef TAN_SRC_AST_AST_FUNC_H_
#define TAN_SRC_AST_AST_FUNC_H_
#include "src/ast/astnode.h"
#include "src/llvm_include.h"

namespace tanlang {

struct Token;

class ASTFunction;

using ASTFunctionPtr = std::shared_ptr<ASTFunction>;

/**
 * return type, function name, arg1, arg2, ...
 * */
class ASTFunction final : public ASTNode, public std::enable_shared_from_this<ASTFunction> {
public:
  ASTFunction(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;
  Value *codegen_prototype(CompilerSession *compiler_session, bool import = false);

  bool is_named() const override { return true; }

  ASTNodePtr get_ret() const;
  std::string get_name() const override;
  ASTNodePtr get_arg(size_t i) const;
  size_t get_n_args() const;
  Function *get_func() const;

protected:
  size_t nud(Parser *parser) override;

private:
  bool _is_external = false;
  bool _is_public = false;
  Function *_func = nullptr;
};

/**
 * arg1, arg2, ...
 * */
class ASTFunctionCall final : public ASTNode {
public:
  ASTFunctionCall() = delete;
  ASTFunctionCall(Token *token, size_t token_index);
  Value *codegen(CompilerSession *compiler_session) override;

protected:
  size_t nud(Parser *parser) override;

public:
  std::string _name{};
};

} // namespace tanlang

#endif /* TAN_SRC_AST_AST_FUNC_H_ */
