#ifndef TAN_PARSEDEF_H
#define TAN_PARSEDEF_H

#include "utils.h"
#include <unordered_map>

namespace tanlang {
enum PrecedenceLevel {
  PREC_LOWEST,
  PREC_ASSIGN = 90,       // = *= /= %= += -= <<= >>= &= ^= |=
  PREC_TERNARY = 100,     // ?:
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
  NUM_LITERAL,
  STRING_LITERAL,
  EOF_,
};

static std::unordered_map<ASTType, std::string> ast_type_names{
    {ASTType::PROGRAM, "PROGRAM"},
    {ASTType::SUM, "SUM"},
    {ASTType::SUBTRACT, "SUBTRACT"},
    {ASTType::MULTIPLY, "MULTIPLY"},
    {ASTType::DIVIDE, "DIVIDE"},
    {ASTType::NUM_LITERAL, "NUM_LITERAL"},
    {ASTType::STRING_LITERAL, "STRING_LITERAL"},
};

// operator precedence for each token
static constexpr const_map<ASTType, int, 8>
    op_precedence(std::pair(ASTType::PROGRAM, PREC_LOWEST), std::pair(ASTType::EOF_, PREC_LOWEST),
                  std::pair(ASTType::SUM, PREC_TERM), std::pair(ASTType::SUBTRACT, PREC_TERM),
                  std::pair(ASTType::MULTIPLY, PREC_FACTOR), std::pair(ASTType::DIVIDE, PREC_FACTOR),
                  std::pair(ASTType::NUM_LITERAL, PREC_LOWEST),
                  std::pair(ASTType::STRING_LITERAL, PREC_LOWEST));
} // namespace tanlang
#endif /* TAN_PARSEDEF_H */
