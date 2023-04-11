#ifndef __TAN_INCLUDE_AST_AST_VISITOR_H__
#define __TAN_INCLUDE_AST_AST_VISITOR_H__

#include "base.h"
#include "ast/ast_base.h"

namespace tanlang {

class Program;
class Identifier;
class VarRef;
class Parenthesis;
class If;
class VarDecl;
class ArgDecl;
class Return;
class CompoundStmt;
class BinaryOrUnary;
class BinaryOperator;
class UnaryOperator;
class Cast;
class Assignment;
class FunctionCall;
class FunctionDecl;
class Import;
class Intrinsic;
class ArrayLiteral;
class CharLiteral;
class BoolLiteral;
class IntegerLiteral;
class FloatLiteral;
class StringLiteral;
class NullPointerLiteral;
class MemberAccess;
class StructDecl;
class Loop;
class BreakContinue;

#ifndef DEFINE_AST_VISITOR_INTERFACE
#define DEFINE_AST_VISITOR_INTERFACE(AST_NAME)                                     \
  /** The interface VisitXX calls this->VisitXXImpl() if defined by Derived */     \
  void Visit##AST_NAME(AST_NAME *p) {                                              \
    constexpr bool has_func = requires(Derived t) { t.Visit##AST_NAME##Impl(p); }; \
    if constexpr (has_func) {                                                      \
      ((Derived *)this)->Visit##AST_NAME##Impl(p);                                 \
    } else {                                                                       \
      ((Derived *)this)->default_visit(p);                                         \
    }                                                                              \
  }
#endif

#ifndef CALL_AST_VISITOR
#define CALL_AST_VISITOR(AST_NAME, NODE) Visit##AST_NAME(ast_cast<AST_NAME>(NODE))
#endif

template <typename Derived> class ASTVisitor {
public:
  DEFINE_AST_VISITOR_INTERFACE(Program)
  DEFINE_AST_VISITOR_INTERFACE(Identifier)
  DEFINE_AST_VISITOR_INTERFACE(Parenthesis)
  DEFINE_AST_VISITOR_INTERFACE(If)
  DEFINE_AST_VISITOR_INTERFACE(VarDecl)
  DEFINE_AST_VISITOR_INTERFACE(ArgDecl)
  DEFINE_AST_VISITOR_INTERFACE(Return)
  DEFINE_AST_VISITOR_INTERFACE(CompoundStmt)
  DEFINE_AST_VISITOR_INTERFACE(BinaryOrUnary)
  DEFINE_AST_VISITOR_INTERFACE(BinaryOperator)
  DEFINE_AST_VISITOR_INTERFACE(UnaryOperator)
  DEFINE_AST_VISITOR_INTERFACE(Cast)
  DEFINE_AST_VISITOR_INTERFACE(Assignment)
  DEFINE_AST_VISITOR_INTERFACE(FunctionCall)
  DEFINE_AST_VISITOR_INTERFACE(FunctionDecl)
  DEFINE_AST_VISITOR_INTERFACE(Import)
  DEFINE_AST_VISITOR_INTERFACE(Intrinsic)
  DEFINE_AST_VISITOR_INTERFACE(ArrayLiteral)
  DEFINE_AST_VISITOR_INTERFACE(CharLiteral)
  DEFINE_AST_VISITOR_INTERFACE(BoolLiteral)
  DEFINE_AST_VISITOR_INTERFACE(IntegerLiteral)
  DEFINE_AST_VISITOR_INTERFACE(FloatLiteral)
  DEFINE_AST_VISITOR_INTERFACE(StringLiteral)
  DEFINE_AST_VISITOR_INTERFACE(NullPointerLiteral)
  DEFINE_AST_VISITOR_INTERFACE(MemberAccess)
  DEFINE_AST_VISITOR_INTERFACE(StructDecl)
  DEFINE_AST_VISITOR_INTERFACE(Loop)
  DEFINE_AST_VISITOR_INTERFACE(BreakContinue)
  DEFINE_AST_VISITOR_INTERFACE(VarRef)

  void visit(ASTBase *p) {
    TAN_ASSERT(p);

    switch (p->get_node_type()) {
    case ASTNodeType::PROGRAM:
      CALL_AST_VISITOR(Program, p);
      break;
    case ASTNodeType::COMPOUND_STATEMENT:
      CALL_AST_VISITOR(CompoundStmt, p);
      break;
    case ASTNodeType::RET:
      CALL_AST_VISITOR(Return, p);
      break;
    case ASTNodeType::IF:
      CALL_AST_VISITOR(If, p);
      break;
    case ASTNodeType::LOOP:
      CALL_AST_VISITOR(Loop, p);
      break;
    case ASTNodeType::BREAK:
    case ASTNodeType::CONTINUE:
      CALL_AST_VISITOR(BreakContinue, p);
      break;
    case ASTNodeType::IMPORT:
      CALL_AST_VISITOR(Import, p);
      break;
      /// expressions
    case ASTNodeType::ASSIGN:
      CALL_AST_VISITOR(Assignment, p);
      break;
    case ASTNodeType::CAST:
      CALL_AST_VISITOR(Cast, p);
      break;
    case ASTNodeType::BOP:
      CALL_AST_VISITOR(BinaryOperator, p);
      break;
    case ASTNodeType::UOP:
      CALL_AST_VISITOR(UnaryOperator, p);
      break;
    case ASTNodeType::BOP_OR_UOP:
      CALL_AST_VISITOR(BinaryOrUnary, p);
      break;
    case ASTNodeType::ID:
      CALL_AST_VISITOR(Identifier, p);
      break;
    case ASTNodeType::STRING_LITERAL:
      CALL_AST_VISITOR(StringLiteral, p);
      break;
    case ASTNodeType::CHAR_LITERAL:
      CALL_AST_VISITOR(CharLiteral, p);
      break;
    case ASTNodeType::BOOL_LITERAL:
      CALL_AST_VISITOR(BoolLiteral, p);
      break;
    case ASTNodeType::INTEGER_LITERAL:
      CALL_AST_VISITOR(IntegerLiteral, p);
      break;
    case ASTNodeType::FLOAT_LITERAL:
      CALL_AST_VISITOR(FloatLiteral, p);
      break;
    case ASTNodeType::ARRAY_LITERAL:
      CALL_AST_VISITOR(ArrayLiteral, p);
      break;
    case ASTNodeType::NULLPTR_LITERAL:
      CALL_AST_VISITOR(NullPointerLiteral, p);
      break;
    case ASTNodeType::INTRINSIC:
      CALL_AST_VISITOR(Intrinsic, p);
      break;
    case ASTNodeType::PARENTHESIS:
      CALL_AST_VISITOR(Parenthesis, p);
      break;
    case ASTNodeType::FUNC_CALL:
      CALL_AST_VISITOR(FunctionCall, p);
      break;
    case ASTNodeType::FUNC_DECL:
      CALL_AST_VISITOR(FunctionDecl, p);
      break;
    case ASTNodeType::ARG_DECL:
      CALL_AST_VISITOR(ArgDecl, p);
      break;
    case ASTNodeType::VAR_DECL:
      CALL_AST_VISITOR(VarDecl, p);
      break;
    case ASTNodeType::STRUCT_DECL:
      CALL_AST_VISITOR(StructDecl, p);
      break;
    case ASTNodeType::VAR_REF:
      CALL_AST_VISITOR(VarRef, p);
      break;
    default:
      TAN_ASSERT(false);
    }
  }

  ~ASTVisitor() = default;
  virtual void default_visit(ASTBase *) {}
};

#undef DEFINE_AST_VISITOR_INTERFACE

#ifndef DEFINE_AST_VISITOR_IMPL
#define DEFINE_AST_VISITOR_IMPL(CLASS, AST_NAME) void CLASS::Visit##AST_NAME##Impl(AST_NAME *p)
#endif

#ifndef DECLARE_AST_VISITOR_IMPL
#define DECLARE_AST_VISITOR_IMPL(AST_NAME) void Visit##AST_NAME##Impl(AST_NAME *p)
#endif

} // namespace tanlang

#endif //__TAN_INCLUDE_AST_AST_VISITOR_H__
