#ifndef __TAN_SRC_AST_AST_BUILDER_H__
#define __TAN_SRC_AST_AST_BUILDER_H__
#include "src/ast/fwd.h"
#include "base.h"

namespace tanlang {

/**
 * \brief Utility used to create AST nodes with appropriate attributes (mostly type) filled.
 */
class ASTBuilder {
public:
  static IntegerLiteral *CreateIntegerLiteral(CompilerSession *cs, uint64_t val, bool is_unsigned);
  static FloatLiteral *CreateFloatLiteral(CompilerSession *cs, double val);
  static StringLiteral *CreateStringLiteral(CompilerSession *cs, str val);
  static CharLiteral *CreateCharLiteral(CompilerSession *cs, uint8_t val);
  static ArrayLiteral *CreateArrayLiteral(CompilerSession *cs, ASTType *element_type);

};

}

#endif //__TAN_SRC_AST_AST_BUILDER_H__
