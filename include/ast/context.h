#ifndef __TAN_CONTEXT_H__
#define __TAN_CONTEXT_H__
#include "base.h"

namespace tanlang {

class ASTBase;
class Decl;
class FunctionDecl;

class Context {
public:
  Context &operator=(const Context &) = delete;
  Context(const Context &) = delete;
  Context() = delete;
  explicit Context(ASTBase *owner);

public:
  /**
   * \brief Register a type declaration
   */
  void set_decl(const str &name, Decl *decl);

  /**
   * \brief Search for a declaration by name
   */
  Decl *get_decl(const str &name) const;

  /**
   * \brief Get all type declarations in the context
   */
  vector<Decl *> get_decls() const;

  /**
   * \brief Register a function declaration
   */
  void set_function_decl(FunctionDecl *func);

  /**
   * \brief Search for a function declaration by name
   */
  FunctionDecl *get_func_decl(const str &name) const;

  /**
   * \brief Get all functions registered in the context
   */
  vector<FunctionDecl *> get_func_decls() const;

  ASTBase *owner() const;

private:
  umap<str, Decl *> _type_decls{};
  umap<str, FunctionDecl *> _func_decls{};
  ASTBase *_owner = nullptr;
};

} // namespace tanlang

#endif //__TAN_CONTEXT_H__
