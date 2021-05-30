#ifndef __TAN_SRC_AST_AST_NODE_TYPE_H__
#define __TAN_SRC_AST_AST_NODE_TYPE_H__

namespace tanlang {

enum class ASTNodeType {
  PROGRAM,
  FUNC_DECL,
  FUNC_CALL,
  ARG_DECL,
  VAR_DECL,
  STRUCT_DECL,
  ENUM_DECL,
  ENUM_VAL,
  STATEMENT,   /// statement or compound statements
  BOP,         /// binary operators, see BinaryOpKind
  ADDRESS_OF,  /// &
  CAST,        /// as
  ID,          /// identifiers
  LOOP,        /// for, while, ...
  CONTINUE,    /// continue
  BREAK,       /// break
  TY,          /// type name
  PARENTHESIS, /// ()
  RET,         /// return
  IF,          /// if
  ELSE,        /// else
  IMPORT,      /// import

  // types in tan
  INTEGER_LITERAL, /// int literal
  FLOAT_LITERAL,   /// float literal
  CHAR_LITERAL,    /// 's'
  STRING_LITERAL,  /// "xxx"
  ARRAY_LITERAL,   /// [1, 2, 3]
  MEMBER_ACCESS,   /// struct.a
  INTRINSIC,       /// intrinsic functions, operators, qualifiers, etc.
  INVALID,
};

}

#endif //__TAN_SRC_AST_AST_NODE_TYPE_H__
