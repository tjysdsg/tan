#ifndef __TAN_SRC_AST_AST_BUILDER_H__
#define __TAN_SRC_AST_AST_BUILDER_H__
#include "src/ast/fwd.h"
#include "src/ast/source_manager.h"
#include "base.h"

namespace tanlang {

class Type;

// TODO IMPORTANT: move this into Literal
/**
 * \brief Utility used to create AST nodes with appropriate attributes filled.
 */
class ASTBuilder {
public:
  static IntegerLiteral *CreateIntegerLiteral(SrcLoc loc, uint64_t val, size_t bit_size, bool is_unsigned);
  static BoolLiteral *CreateBoolLiteral(SrcLoc loc, bool val);
  static FloatLiteral *CreateFloatLiteral(SrcLoc loc, double val, size_t bit_size);
  static StringLiteral *CreateStringLiteral(SrcLoc loc, str val);
  static CharLiteral *CreateCharLiteral(SrcLoc loc, uint8_t val);
  static ArrayLiteral *CreateArrayLiteral(SrcLoc loc, Type *element_type, int size);
  static NullPointerLiteral *CreateNullPointerLiteral(SrcLoc loc, Type *element_type);
};

}

#endif //__TAN_SRC_AST_AST_BUILDER_H__
