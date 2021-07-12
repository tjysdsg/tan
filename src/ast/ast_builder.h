#ifndef __TAN_SRC_AST_AST_BUILDER_H__
#define __TAN_SRC_AST_AST_BUILDER_H__
#include "src/ast/fwd.h"
#include "src/ast/source_manager.h"
#include "base.h"

namespace tanlang {

/**
 * \brief Utility used to create AST nodes with appropriate attributes (mostly type) filled.
 */
class ASTBuilder {
public:
  static IntegerLiteral *CreateIntegerLiteral(ASTContext *ctx,
      SourceIndex loc,
      uint64_t val,
      size_t bit_size,
      bool is_unsigned);
  static BoolLiteral *CreateBoolLiteral(ASTContext *ctx, SourceIndex loc, bool val);
  static FloatLiteral *CreateFloatLiteral(ASTContext *ctx, SourceIndex loc, double val, size_t bit_size);
  static StringLiteral *CreateStringLiteral(ASTContext *ctx, SourceIndex loc, str val);
  static CharLiteral *CreateCharLiteral(ASTContext *ctx, SourceIndex loc, uint8_t val);
  static ArrayLiteral *CreateArrayLiteral(ASTContext *ctx, SourceIndex loc, ASTType *element_type);
};

}

#endif //__TAN_SRC_AST_AST_BUILDER_H__
