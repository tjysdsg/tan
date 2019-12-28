#ifndef TAN_PARSEDEF_H
#define TAN_PARSEDEF_H

#include <unordered_map>

namespace tanlang {
enum PrecedenceLevel {
  PREC_LOWEST,
  PREC_LITERAL = 10,
  PREC_ASSIGN = 90,       // = *= /= %= += -= <<= >>= &= ^= |=
  PREC_LOGICAL_OR = 110,  // ||
  PREC_LOGICAL_AND = 120, // &&
  PREC_COMPARISON = 130,  // < <= > >= == != === !== ~=
  PREC_ISA = 132,         // isa
  PREC_RANGE = 135,       // ..< ...
  PREC_TERM = 140,        // + - | ^
  PREC_FACTOR = 150,      // * / % &
  PREC_SHIFT = 160,       // << >>
  PREC_UNARY = 170,       // + - ! ~
  PREC_CALL = 200         // . ( [
};

enum class ASTType {
  PROGRAM,

  SUM,
  SUBTRACT,
  MULTIPLY,
  DIVIDE,
  MOD,
  ASSIGN,

  BAND, // binary and
  LAND, // logical and
  BOR,  // binary or
  LOR,  // logical or
  BNOT, // binary not
  LNOT, // logical not
  XOR,

  NUM_LITERAL,
  STRING_LITERAL,
  EOF_,
};

extern std::unordered_map<ASTType, std::string> ast_type_names;

// operator precedence for each token
extern std::unordered_map<ASTType, int> op_precedence;

} // namespace tanlang
#endif /* TAN_PARSEDEF_H */
