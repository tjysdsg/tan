#ifndef __TAN_SRC_AST_AST_CONTEXT_H__
#define __TAN_SRC_AST_AST_CONTEXT_H__
#include "base.h"

namespace tanlang {

class ASTContext {
public:
  static void AddPublicFunction(const str &filename, FunctionDecl *func);
  static vector<FunctionDecl *> GetPublicFunctions(const str &filename);

private:
  /**
   * \brief Function table for each source files
   * \details filename -> (function name -> FunctionTable)
   * */
  static inline umap<str, FunctionTable *> public_func{};

public:
  ASTContext &operator=(const ASTContext &) = delete;
  ASTContext(const ASTContext &) = delete;
  ASTContext() = delete;
  explicit ASTContext(str filename);
  ~ASTContext();

public:
  SourceManager *get_source_manager() const;
  void set_source_manager(SourceManager *sm);
  str get_source_location_str(SourceTraceable *p) const;

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
   * \brief Set the current scope
   * \see Scope
   * */
  void push_scope(Scope *scope);

  /**
   * \brief Pop the current scope
   * \see Scope
   * */
  Scope *pop_scope();

  /**
   * \brief Add a named ASTNode so that others can loop it up using CompilerSession::get
   * TODO: ASTBase -> Decl
   * */
  void add(const str &name, ASTBase *value);

  /**
   * \brief look up the variable table in the current and parent scopes
   * \details This function starts by searching the current scope. If the target is not found in current scope,
   * search the parent scope, repeat the process until found. Return nullptr if not found in all visible scopes.
   * */
  ASTBase *get(const str &name);

  /**
   * \brief Register a type declaration
   * \note Registers only the original type declaration or a forward declaration,
   *    use CompilerSession::add_type_accessor() to register a type accessor
   * */
  void add_type_decl(const str &name, Decl *decl);

  /**
   * \brief Look up type table
   * \param name typename
   * \note Returns only the original type declaration or a forward declaration, use CompilerSession::get_type_accessor()
   * to get a type accessor
   */
  Decl *get_type_decl(const str &name);

  /**
   * \brief Add a function AST to the current file's function table
   * \details This will not add anything to the public function table, to do that,
   * call CompilerSession::AddPublicFunction
   * */
  void add_function(FunctionDecl *func);
  vector<FunctionDecl *> get_functions(const str &name);
  [[nodiscard]] Loop *get_current_loop() const;
  void set_current_loop(Loop *loop);
  const str &get_filename() const;

public:
  str _filename = "";

private:
  umap<str, Decl *> _type_decls{};
  vector<Scope *> _scope{};
  FunctionTable *_function_table = nullptr;

  /**
   * The control flow in current scope, used by break and continue
   * */
  Loop *_current_loop = nullptr;
  SourceManager *_sm = nullptr;

private:
  void initialize_scope();
};

}

#endif //__TAN_SRC_AST_AST_CONTEXT_H__
