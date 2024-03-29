/**
 * \file Contains forward declarations of all ASTs
 *
 */
#ifndef __TAN_SRC_AST_FWD_H__
#define __TAN_SRC_AST_FWD_H__

namespace tanlang {

class ASTBase;
class Type;
class TypeRef;
class TypeDecl;
class BinaryOperator;
class UnaryOperator;
class MemberAccess;
class Assignment;
class Cast;
class Decl;
class Stmt;
class Expr;
class Literal;
class BoolLiteral;
class IntegerLiteral;
class FloatLiteral;
class StringLiteral;
class ArrayLiteral;
class NullPointerLiteral;
class CharLiteral;
class FunctionDecl;
class CompoundStmt;
class Loop;
class SourceTraceable;
class ASTNamed;
class Token;
class SrcLoc;
class TokenizedSourceFile;
class Intrinsic;
class If;
class VarRef;
class Constructor;
class CompTimeExpr;

} // namespace tanlang

#endif //__TAN_SRC_AST_FWD_H__
