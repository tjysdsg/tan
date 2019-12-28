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

    Parser::~Parser() {
        delete _root;
        _root = nullptr;
    }

    void ASTNode::printSubtree(const std::string &prefix) {
        using std::cout;
        using std::endl;
        if (_children.empty()) return;
        cout << prefix;
        size_t n_children = _children.size();
        cout << (n_children > 1 ? "├── " : "");

        for (size_t i = 0; i < n_children; ++i) {
            ASTNode *c = _children[i];
            if (i < n_children - 1) {
                bool printStrand = n_children > 1 && !c->_children.empty();
                std::string newPrefix = prefix + (printStrand ? "│\t" : "\t");
                std::cout << ast_type_names[c->_op] << "\n";
                c->printSubtree(newPrefix);
            } else {
                cout << (n_children > 1 ? prefix : "") << "└── ";
                std::cout << ast_type_names[c->_op] << "\n";
                c->printSubtree(prefix + "\t");
            }
        }
    }

    void ASTNode::printTree() {
        using std::cout;
        std::cout << ast_type_names[this->_op] << "\n";
        printSubtree("");
        cout << "\n";
    }

    ASTNode *ASTInfixBinaryOp::led(ASTNode *left, Parser *parser) {
        _children.emplace_back(left);
        auto *n = parser->next_expression(_lbp);
        if (!n) {
            // # TODO: report error
            throw "SHIT";
        } else {
            _children.emplace_back(n);
        }
        return this;
    }

    ASTNode *ASTInfixBinaryOp::nud(Parser *parser) {
        assert(false);
        return nullptr;
    }
}
