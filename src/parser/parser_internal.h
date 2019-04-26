#ifndef __TAN_SRC_PARSER_PARSER_INTERNAL_H__
#define __TAN_SRC_PARSER_PARSER_INTERNAL_H__
#include <cctype>
#include <vector>

namespace tanlang {
    // AST_NODE_TYPE = (AST_NODE_SUB_TYPE << 32) | AST_NODE_BASE_TYPE;
    typedef uint64_t AST_NODE_TYPE;
#define MAKE_AST_NODE_TYPE(sub, base)                                          \
    ((((uint64_t)(sub)) << 32) | (uint64_t)(base))

#define GET_AST_NODE_SUB_TYPE(type) ((uint32_t)((type) >> 32u))
#define GET_AST_NODE_BASE_TYPE(type) ((uint32_t)((type)&0x00000000ffffffff))

    enum AST_NODE_BASE_TYPE : uint32_t {
        PROGRAM = 0,
        EXPR = 1u,
        STMT = 1u << 2u,
        UOP = 1u << 3u, // unary operators
        BOP = 1u << 4u, // binary operators
    };
    enum AST_NODE_SUB_TYPE : uint32_t {
        // expressions
        CAST_EXPR = 1,
        SIZEOF_EXPR,
        LITERAL_EXPR,
        IDENTIFIER,
        EXPR0,  // see syntax.md
        EXPR1,  // see syntax.md
        EXPR2,  // see syntax.md
        EXPR3,  // see syntax.md
        EXPR4,  // see syntax.md
        EXPR5,  // see syntax.md
        EXPR6,  // see syntax.md
        EXPR7,  // see syntax.md
        EXPR8,  // see syntax.md
        EXPR9,  // see syntax.md
        EXPR10, // see syntax.md
        EXPR11, // see syntax.md
        EXPR12, // see syntax.md
        EXPR13, // see syntax.md
        EXPR14, // see syntax.md
        EXPR15, // see syntax.md
        // statements
        ASSIGN,
        SWITCH_CASE,
        SWITCH_BLOCK,
        STMT_BLOCK,
        IF,
        WHILE,
        FOR,
        DO_WHILE,
        SWITCH,
        DECL,
        EXPR_STMT, // expr + ';'
        // unary operators
        POS,
        NEG,
        LNOT,      // logical not
        BNOT,      // bitwise not
        ADR_REF,   // address reference
        ADR_DEREF, // address dereference
        // binary operators
        LSHIFT, // left shift
        RSHIFT, // right shift
        LEQ,    // logical equal
        LNE,    // logical not equal
        LLT,    // logical less than
        LLE,    // logical less or equal
        LGT,    // logical greater than
        LGE,    // logical greater or equal
        LAND,   // logical and
        BAND,   // bitwise and
        LOR,    // logical or
        BOR,    // bitwise or
        BXOR,   // bitwise xor
        ADD,    // addition
        SUB,    // subtraction
        MUL,    // multiplication
        DIV,    // division
        MOD,    // remain
        // assignment operators
        ADD_ASSIGN,
        SUB_ASSIGN,
        BOR_ASSIGN,
        BXOR_ASSIGN,
        BAND_ASSIGN,
        LSHIFT_ASSIGN,
        RSHIFT_ASSIGN,
        MUL_ASSIGN,
        DIV_ASSIGN,
        MOD_ASSIGN,
    };

    struct ast_node {
        AST_NODE_TYPE type;
        std::vector<ast_node *> children;
        union {
            std::string str{};
            uint64_t val;
            double fval;
        };
        ast_node(){};
        ~ast_node(){};
    };
} // namespace tanlang

#endif // __TAN_SRC_PARSER_PARSER_INTERNAL_H__
