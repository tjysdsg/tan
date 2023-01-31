#ifndef __TAN_SRC_AST_AST_CONTEXT_H__
#define __TAN_SRC_AST_AST_CONTEXT_H__
#include "base.h"
#include "ast/function_table.h"
#include "include/ast/fwd.h"

namespace tanlang {

class ASTContext {
private:
  static inline umap<str, ASTContext *> file_to_ctx{};

public:
  static ASTContext *get_ctx_of_file(const str &filename);

public:
  ASTContext &operator=(const ASTContext &) = delete;
  ASTContext(const ASTContext &) = delete;
  ASTContext() = delete;
  ASTContext(SourceManager *sm);
  ~ASTContext();

public:
  SourceManager *get_source_manager() const;
  str get_source_location_str(SourceTraceable *p) const;
  str get_filename() const;

  /**
   * \brief Get current scope
   * \see Scope
   * */
  Scope *get_current_scope();

  /**
   * \brief create_ty a new scope
   * \see Scope
   * */
  Scope *push_scope();

  /**
   * \brief Pop the current scope
   * \see Scope
   * */
  Scope *pop_scope();

  /**
   * \brief Find the loop in the outer-est scope
   * \return nullptr if not found
   */
  [[nodiscard]] Loop *get_current_loop() const;

  /**
   * \brief Set the loop in current scope
   */
  void set_current_loop(Loop *loop);

  /**
   * \brief Register a declaration in the current scope
   */
  void add_scoped_decl(const str &name, Decl *value);

  /**
   * \brief Look up a declaration the current and parent scopes
   * \details This function starts by searching the current scope. If the target is not found in current scope,
   * search the parent scope, repeat the process until found. Return nullptr if not found in all visible scopes.
   */
  Decl *get_scoped_decl(const str &name);

  /**
   * \brief Register a type declaration
   */
  void add_type_decl(const str &name, TypeDecl *decl, bool is_public = false);

  /**
   * \brief Look up type table
   * \param name typename
   */
  TypeDecl *get_type_decl(const str &name);

  /**
   * \brief Look up type table
   * \param name typename
   */
  vector<TypeDecl *> get_public_type_decls();

  /**
   * \brief Add a function AST to the current file's function table
   * \details This will not add_scoped_decl anything to the public function table
   */
  void add_function_decl(FunctionDecl *func, bool is_public = false);

  /**
   * \brief Add a function AST to the current file's function table
   * \details This will not add_scoped_decl anything to the public function table
   */
  vector<FunctionDecl *> get_public_functions();

  /**
   * \brief Search for a function declaration with specified name
   * \param name Name
   * \param include_imported Whether to search in package-level function table
   */
  vector<FunctionDecl *> get_functions(const str &name, bool include_imported = true);

private:
  vector<Scope *> _scope{};

  /**
   * \brief Type declarations that are only locally visible
   */
  umap<str, TypeDecl *> _private_type_decls{};

  /**
   * \brief Function declarations that are only locally visible
   */
  FunctionTable _private_function_table{};

  /**
   * \brief Type declarations that are visible to other files
   */
  umap<str, TypeDecl *> _public_type_decls{};

  /**
   * \brief Function declarations that are visible to other files
   */
  FunctionTable _public_function_table{};

  SourceManager *_sm = nullptr;

private:
  void initialize_scope();
};

} // namespace tanlang

#endif //__TAN_SRC_AST_AST_CONTEXT_H__
