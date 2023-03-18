#ifndef __TAN_CONTEXT_H__
#define __TAN_CONTEXT_H__
#include "base.h"
#include "ast/function_table.h"
#include "include/ast/fwd.h"

namespace tanlang {

class Context {
public:
  Context &operator=(const Context &) = delete;
  Context(const Context &) = delete;
  Context() = delete;
  Context(ASTBase *owner);

public:
  /**
   * \brief Register a type declaration
   */
  void add_type_decl(const str &name, TypeDecl *decl);

  /**
   * \brief Search for a type declaration by name
   */
  TypeDecl *get_type_decl(const str &name);

  /**
   * \brief Get all type declarations in the context
   */
  vector<TypeDecl *> get_type_decls();

  /**
   * \brief Register a function declaration
   */
  void add_function_decl(FunctionDecl *func);

  /**
   * \brief Search for a function declaration by name
   */
  vector<FunctionDecl *> get_functions(const str &name);

private:
  umap<str, TypeDecl *> _type_decls{};
  FunctionTable _function_table{};
  ASTBase *_owner = nullptr;
};

} // namespace tanlang

#endif //__TAN_CONTEXT_H__
