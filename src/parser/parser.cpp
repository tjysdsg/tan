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
    bool parse##type(ast_node *&new_node, const std::vector<token_info *> &ts, \
                     size_t &curr_idx)

    // parser function prototypes
    // TODO: implement all parser functions
    DEFINE_PARSER_FUNCTION_FOR_TYPE(KEYWORD);
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
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(STMT);
    // parser function for keywords
    DEFINE_PARSER_FUNCTION_FOR_TYPE(IF) {
        // (
        if (ts[++curr_idx]->type != OPEN_PAREN) {
            return false;
        }
        // condition in if
        ast_node *new_child_node = nullptr;
        if (parseEXPR(new_child_node, ts, ++curr_idx)) {
            return false;
        }
        new_node->children.push_back(new_child_node);
        // )
        if (ts[++curr_idx]->type != CLOSE_PAREN) {
            return false;
        }

        // code block in if
        if (ts[++curr_idx]->type != OPEN_BRACE) {
            return false;
        }
        new_child_node = nullptr;
        if (parseSTMT(new_child_node, ts, ++curr_idx)) {
            return false;
        }
        new_node->children.push_back(new_child_node);

        if (ts[++curr_idx]->type != CLOSE_BRACE) {
            return false;
        }
        // else
        if (ts[curr_idx + 1]->str == "else") {
            ++curr_idx;
            // code block in else
            if (ts[++curr_idx]->type != OPEN_BRACE) {
                return false;
            }
            new_child_node = nullptr;
            if (parseSTMT(new_child_node, ts, ++curr_idx)) {
                return false;
            }
            new_node->children.push_back(new_child_node);
            if (ts[++curr_idx]->type != CLOSE_BRACE) {
                return false;
            }
        }
        return true;
    }

    DEFINE_PARSER_FUNCTION_FOR_TYPE(FOR);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(WHILE);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(VARDECL);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(CONSTDECL);
    DEFINE_PARSER_FUNCTION_FOR_TYPE(FNDECL);

    DEFINE_PARSER_FUNCTION_FOR_TYPE(KEYWORD) {
        auto *t = ts[curr_idx];
        // lexer set the type of all keywords as ID, since it's parser's job to
        // recognize them
        if (t->type == ID) {
            new_node = new ast_node;
            if (t->str == "if") {
                new_node->type = MAKE_AST_NODE_TYPE(IF, STMT);
                return parseIF(new_node, ts, curr_idx);
            } else if (t->str == "for") {
                new_node->type = MAKE_AST_NODE_TYPE(FOR, STMT);
                return parseFOR(new_node, ts, curr_idx);
            } else if (t->str == "while") {
                new_node->type = MAKE_AST_NODE_TYPE(WHILE, STMT);
                return parseWHILE(new_node, ts, curr_idx);
            } else if (t->str == "var") {
                new_node->type = MAKE_AST_NODE_TYPE(VARDECL, STMT);
                return parseVARDECL(new_node, ts, curr_idx);
            } else if (t->str == "let") {
                new_node->type = MAKE_AST_NODE_TYPE(CONSTDECL, STMT);
                return parseCONSTDECL(new_node, ts, curr_idx);
            } else if (t->str == "enum") {
            } else if (t->str == "fn") {
                new_node->type = MAKE_AST_NODE_TYPE(FNDECL, STMT);
                return parseFNDECL(new_node, ts, curr_idx);
            } else if (t->str == "typename") {
            } else if (t->str == "struct") {
            } else if (t->str == "union") {
            } else if (t->str == "case") {
            } else if (t->str == "default") {
            } else if (t->str == "do") {
            } else if (t->str == "switch") {
            } else if (t->str == "break") {
            } else if (t->str == "continue") {
            } else if (t->str == "return") {
            } else {
                return false;
            }
        }
        return true;
    }
    // definitions of parser functions
    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR) {
        typedef bool (*parse_func_t)(
            ast_node *&, const std::vector<token_info *> &, size_t &);
        std::vector<parse_func_t> expr_parsers = {
            parseEXPR0,  parseEXPR1,  parseEXPR2,  parseEXPR3,
            parseEXPR4,  parseEXPR5,  parseEXPR6,  parseEXPR7,
            parseEXPR8,  parseEXPR9,  parseEXPR10, parseEXPR11,
            parseEXPR12, parseEXPR13, parseEXPR14, parseEXPR15,
        };
        auto new_idx = curr_idx + 1;
        ast_node *new_child_node = nullptr;
        for (parse_func_t pfn : expr_parsers) {
            if (pfn(new_child_node, ts, new_idx)) {
                curr_idx = new_idx;
                new_node->children.push_back(new_child_node);
                return true;
            }
            // TODO: error handling
        }
        return false;
    }

    DEFINE_PARSER_FUNCTION_FOR_TYPE(IDENTIFIER) {
        auto *t = ts[curr_idx];
        if (t->type == ID) {
            new_node = new ast_node;
            new_node->type = MAKE_AST_NODE_TYPE(IDENTIFIER, EXPR);
            new_node->str = t->str;
            return true;
        }
        return false;
    }

    DEFINE_PARSER_FUNCTION_FOR_TYPE(EXPR0) {
        auto *t = ts[curr_idx];
        bool success = false;
        ast_node *child_node = nullptr;
        if (parseIDENTIFIER(child_node, ts,
                            curr_idx)) { // expr0 = IDENTIFIER
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

    Parser::~Parser() { free_ast_node(_ast); }

    void Parser::parse(Lexer const *lexer) {
        typedef bool (*parse_func_t)(
            ast_node *&, const std::vector<token_info *> &, size_t &);
        std::vector<parse_func_t> parse_functions = {
            parseKEYWORD,
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
