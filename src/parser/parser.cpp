#include "parser.h"
#include <vector>

using std::string;

namespace tanlang {

    ASTNode *Parser::advance() {
        if (_curr_token >= _tokens.size()) return nullptr;
        Token *token = _tokens[_curr_token++];
        ASTNode *node = nullptr;
        if (token->value == "+" && token->type == TokenType::BOP) {
            node = new ASTSum;
        } else if (token->value == "-" && token->type == TokenType::BOP) {
            node = new ASTSubtract;
        } else if (token->value == "*" && token->type == TokenType::BOP) {
            node = new ASTMultiply;
        } else if (token->value == "/" && token->type == TokenType::BOP) {
            node = new ASTDivide;
        } else if (token->type == TokenType::INT) {
            node = new ASTNumberLiteral(token->value, false);
        } else if (token->type == TokenType::FLOAT) {
            node = new ASTNumberLiteral(token->value, true);
        } else if (token->type == TokenType::STRING) {
            node = new ASTStringLiteral(token->value);
        } else {
            // TODO: report error
            throw "FUCK";
        }
        return node;
    }

    ASTNode *Parser::peek() {
        auto *r = advance();
        --_curr_token;
        return r;
    }

    ASTNode *Parser::next_expression(int rbp) {
        ASTNode *node = advance();
        if (!node) {
            return nullptr;
        }
        auto *n = node;
        node = advance();
        if (!node) {
            return n;
        }
        ASTNode *left = n->nud(this);
        while (rbp < node->_lbp) {
            n = node;
            node = peek();
            left = n->led(left, this);
        }
        return left;
    }

    ASTNode *Parser::parse() {
        size_t n_tokens = _tokens.size();
        _root = new ASTNode;
        while (_curr_token < n_tokens) {
            auto *n = next_expression(0);
            if (!n) { break; }
            _root->add(n);
        }
        return _root;
    }
}
