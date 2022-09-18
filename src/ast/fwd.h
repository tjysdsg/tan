/**
 * \file Contains forward declarations of all ASTs
 *
 */
#ifndef __TAN_SRC_AST_FWD_H__
#define __TAN_SRC_AST_FWD_H__

namespace tanlang {

class ASTBase;
class TypeAccessor;
class ASTType;
class BinaryOperator;
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
class FunctionTable;
class Loop;
struct Scope;
class SourceTraceable;
class CompilerSession;
class ASTContext;
class ASTNamed;
class Token;
class SourceIndex;
class SourceManager;
class Intrinsic;
class If;
class Constructor;
class CompTimeExpr;

}

#endif //__TAN_SRC_AST_FWD_H__
