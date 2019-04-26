#include "parser.h"
#include "src/parser/parser_internal.h"
#include "src/lexer/lexer_internal.h"
#include "lexer.h"
#include <iostream>
#include <iomanip>

namespace tanlang {
    void free_ast_node(ast_node *n) {
        if (n == nullptr)
            return;
        for (size_t i = 0; i < n->children.size(); ++i) {
            free_ast_node(n->children[i]);
        }
        delete n;
    }

#define DEFINE_PARSER_FUNCTION_FOR_TYPE(type)                                  \
    bool parse##type(ast_node *&new_node, std::vector<token_info *> ts,        \
                     size_t &curr_idx)

    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR0);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR1);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR2);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR3);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR4);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR5);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR6);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR7);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR8);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR9);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR10);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR11);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR12);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR13);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR14);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR15);

    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR) {
        return !(parseEXPR0(new_node, ts, curr_idx) == false
                 //&&
                 // parseEXPR1(new_node, ts, curr_idx) == false &&
                 // parseEXPR2(new_node, ts, curr_idx) == false &&
                 // parseEXPR3(new_node, ts, curr_idx) == false &&
                 // parseEXPR4(new_node, ts, curr_idx) == false &&
                 // parseEXPR5(new_node, ts, curr_idx) == false &&
                 // parseEXPR6(new_node, ts, curr_idx) == false &&
                 // parseEXPR7(new_node, ts, curr_idx) == false &&
                 // parseEXPR8(new_node, ts, curr_idx) == false &&
                 // parseEXPR9(new_node, ts, curr_idx) == false &&
                 // parseEXPR10(new_node, ts, curr_idx) == false &&
                 // parseEXPR11(new_node, ts, curr_idx) == false &&
                 // parseEXPR12(new_node, ts, curr_idx) == false &&
                 // parseEXPR13(new_node, ts, curr_idx) == false &&
                 // parseEXPR14(new_node, ts, curr_idx) == false &&
                 // parseEXPR15(new_node, ts, curr_idx) == false
        );
    }

    DEFINE_PARSER_FUNCTION_FOR_TYPE(IDENTIFIER) {
        auto *t = ts[curr_idx];
        if (t->type == ID) {
            new_node = new ast_node;
            new_node->type = MAKE_AST_NODE_TYPE(IDENTIFIER, EXPR);
            new_node->str = t->str;
            // TODO: Keyword Recognition
            return true;
        }
        return false;
    }

    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR0) {
        auto *t = ts[curr_idx];
        bool success = false;
        ast_node *child_node = nullptr;
        if (parseIDENTIFIER(child_node, ts, curr_idx)) { // expr0 = IDENTIFIER
            new_node->children.push_back(child_node);
            success = true;
        } else if (t->type == OPEN_PAREN) {
            size_t i = curr_idx;
            while (i < ts.size()) {
                if (ts[i]->type == CLOSE_PAREN) {
                    break;
                }
                ++i;
            }
            if (i == ts.size() - 1) {
                // cannot find the closing parenthesis
                success = false;
            } else {
                if (parseEXPR(child_node, ts,
                              i)) { // expr0 = '(' expr ')'
                    curr_idx = i;
                    new_node->children.push_back(child_node);
                    success = true;
                }
            }
        }
        if (!success) {
            new_node = nullptr;
        }
        return success;
    }

#define PARSER_FUNCTION_SWITCH_CASE_FOR_TYPE(type)                             \
    case type: {                                                               \
        ast_node *new_node = nullptr;                                          \
        while (parse##type(new_node, ts, curr_idx) == false) {}                \
        _ast->children.push_back(new_node);                                    \
    } break

    Parser::~Parser() { free_ast_node(_ast); }

    void Parser::parse(Lexer const *lexer) {
        typedef bool (*parse_func_t)(ast_node *&, std::vector<token_info *>,
                                     size_t &);
        std::vector<parse_func_t> parse_functions = {
            parseIDENTIFIER,
            parseEXPR,
        };
        _ast = new ast_node;
        _ast->type = PROGRAM;
        //
        size_t curr_idx = 0;
        const std::vector<token_info *> ts = *lexer->get_token_stream();
        ast_node *new_node = nullptr;
        while (curr_idx < ts.size()) {
            for (size_t i = 0; i < parse_functions.size(); ++i) {
                if (parse_functions[i](new_node, ts, curr_idx)) {
                    _ast->children.push_back(new_node);
                    break;
                } else if (i == parse_functions.size() - 1) {
                    // FIXME: Error handling
                }
            }
            ++curr_idx;
        }
    }

    ast_node *Parser::get_ast() const { return _ast; }

#ifdef DEBUG_ENABLED
    void Parser::print_ast_node(ast_node *n, int indent) {
        if (n != nullptr) {
            for (auto *c : n->children) {
                print_ast_node(c, indent + 4);
            }
            if (indent) {
                std::cout << std::setw(indent) << ' ';
            }
            std::cout << GET_AST_NODE_SUB_TYPE(n->type) << "|"
                      << GET_AST_NODE_BASE_TYPE(n->type) << "\n";
        }
    }
#endif
} // namespace tanlang
