#ifndef __TAN_SRC_AST_AST_NPTR_H__
#define __TAN_SRC_AST_AST_NPTR_H__
#include "src/ast/astnode.h"
#include "src/ast/ast_struct.h"
#include "src/ast/ast_ty.h"

namespace tanlang {

/**
 * \brief Builtin type: nptr, basically a struct containing a pointer and an int
 * \details Can be used as a regular pointer, except with the ability to do bound checking
 *
 * */
class ASTNPtr : public ASTStruct {
public:
  ASTNPtr() = delete;
  ASTNPtr(Ty orig_type, int n); // FIXME: unsigned? but ASTNumberLiteral
  void nud(Parser *parser) override;

  std::string get_type_name() const override { return _type_name; }

private:
  std::string _type_name = "nptr";
};

} // namespace tanlang
#endif // __TAN_SRC_AST_AST_NPTR_H__
