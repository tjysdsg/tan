#ifndef TAN_PARSEDEF_H
#define TAN_PARSEDEF_H

#include <unordered_map>

namespace tanlang {
enum PrecedenceLevel {
  PREC_LOWEST,
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

#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTType::t, #t}

static std::unordered_map<ASTType, std::string> ast_type_names{
    MAKE_ASTTYPE_NAME_PAIR(PROGRAM),

    MAKE_ASTTYPE_NAME_PAIR(SUM),
    MAKE_ASTTYPE_NAME_PAIR(SUBTRACT),
    MAKE_ASTTYPE_NAME_PAIR(MULTIPLY),
    MAKE_ASTTYPE_NAME_PAIR(DIVIDE),
    MAKE_ASTTYPE_NAME_PAIR(MOD),
    MAKE_ASTTYPE_NAME_PAIR(ASSIGN),

    MAKE_ASTTYPE_NAME_PAIR(NUM_LITERAL),
    MAKE_ASTTYPE_NAME_PAIR(STRING_LITERAL),
    MAKE_ASTTYPE_NAME_PAIR(BAND),
    MAKE_ASTTYPE_NAME_PAIR(LAND),
    MAKE_ASTTYPE_NAME_PAIR(BOR),
    MAKE_ASTTYPE_NAME_PAIR(LOR),
    MAKE_ASTTYPE_NAME_PAIR(BNOT),
    MAKE_ASTTYPE_NAME_PAIR(LNOT),
    MAKE_ASTTYPE_NAME_PAIR(XOR),
};

// operator precedence for each token
static std::unordered_map<ASTType, int>
    op_precedence{
    {ASTType::PROGRAM, PREC_LOWEST},
    {ASTType::EOF_, PREC_LOWEST},

    {ASTType::SUM, PREC_TERM},
    {ASTType::SUBTRACT, PREC_TERM},
    {ASTType::BOR, PREC_TERM},
    {ASTType::XOR, PREC_TERM},

    {ASTType::MULTIPLY, PREC_FACTOR},
    {ASTType::DIVIDE, PREC_FACTOR},
    {ASTType::MOD, PREC_FACTOR},
    {ASTType::BAND, PREC_FACTOR},

    {ASTType::ASSIGN, PREC_ASSIGN},

    {ASTType::BNOT, PREC_UNARY},
    {ASTType::LNOT, PREC_UNARY},

    {ASTType::LAND, PREC_LOGICAL_AND},
    {ASTType::LOR, PREC_LOGICAL_OR},

    {ASTType::NUM_LITERAL, PREC_LOWEST},
    {ASTType::STRING_LITERAL, PREC_LOWEST}
};

} // namespace tanlang
#endif /* TAN_PARSEDEF_H */
