#ifndef __TAN_SRC_AST_PRECEDENCE_H__
#define __TAN_SRC_AST_PRECEDENCE_H__

namespace tanlang {

/**
 * \brief Operator precedence as enums. The higher the value, the higher the precedence
 */
enum PrecedenceLevel {
  PREC_LOWEST = 0,        //
  PREC_LITERAL = 10,      // "string" 1.0 2
  PREC_ASSIGN = 90,       // = *= /= %= += -= <<= >>= &= ^= |=
  PREC_LOGICAL_OR = 110,  // ||
  PREC_LOGICAL_AND = 120, // &&
  PREC_COMPARISON = 130,  // < <= > >= == != === !== ~=
  PREC_RANGE = 135,       // ..< ...
  PREC_TERM = 140,        // + - | ^
  PREC_FACTOR = 150,      // * / % &
  PREC_CAST = 155,        // as
  PREC_SHIFT = 160,       // << >>
  PREC_UNARY = 170,       // + - ! ~
  PREC_CALL = 200,        // . ( [
  PREC_HIGHEST = 500,
};

}

#endif //__TAN_SRC_AST_PRECEDENCE_H__
