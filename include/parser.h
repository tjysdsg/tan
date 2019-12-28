#ifndef TAN_PARSER_H
#define TAN_PARSER_H

#include "base.h"
#include "lexer.h"
#include <unordered_map>
#include "utils.h"
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

namespace tanlang {
    enum class ASTType {
        PROGRAM, SUM, SUBTRACT, MULTIPLY, DIVIDE, NUM_LITERAL, STRING_LITERAL, EOF_,
    };

    static std::unordered_map<ASTType, std::string> ast_type_names{
            {ASTType::PROGRAM,        "PROGRAM"},
            {ASTType::SUM,            "SUM"},
            {ASTType::SUBTRACT,       "SUBTRACT"},
            {ASTType::MULTIPLY,       "MULTIPLY"},
            {ASTType::DIVIDE,         "DIVIDE"},
            {ASTType::NUM_LITERAL,    "NUM_LITERAL"},
            {ASTType::STRING_LITERAL, "STRING_LITERAL"},
    };

    // operator precedence for each token
    static constexpr const_map<ASTType, int, 8> op_precedence(
            std::pair(ASTType::PROGRAM, 0),
            std::pair(ASTType::EOF_, 0),
            std::pair(ASTType::SUM, 10),
            std::pair(ASTType::SUBTRACT, 10),
            std::pair(ASTType::MULTIPLY, 20),
            std::pair(ASTType::DIVIDE, 20),
            std::pair(ASTType::NUM_LITERAL, 0),
            std::pair(ASTType::STRING_LITERAL, 0)
    );

    class ASTNode;

    class Parser final {
    public:
        Parser() = delete;

        explicit Parser(std::vector<Token *> tokens) : _tokens(std::move(tokens)), _curr_token(0) {}

        ASTNode *advance();

        ASTNode *peek();

        ASTNode *next_expression(int rbp = 0);

        ASTNode *parse();

        std::vector<Token *> _tokens;
        ASTNode *_root = nullptr;
        size_t _curr_token;
    };

    class ASTNode {
    public:
        ASTType _op = ASTType::PROGRAM;
        int _associativity{}; // 0 left, 1 non-left
        std::vector<ASTNode *> _children{};
        int _lbp = 0;
        int _rbp = 0;

        ASTNode() = default;

        ASTNode(ASTType op, int associativity, int lbp,
                int rbp) : _op(op), _associativity(associativity), _lbp(lbp), _rbp(rbp) {};

        virtual ~ASTNode() = default;

        [[nodiscard]] virtual int get_ivalue() const {
            assert(false);
            return 0;
        }

        [[nodiscard]] virtual float get_fvalue() const {
            assert(false);
            return 0;
        }

        [[nodiscard]] virtual std::string get_svalue() const {
            assert(false);
            return "";
        }

        [[nodiscard]] virtual ASTNode *led(ASTNode *left, Parser *parser) {
            assert(false);
            return nullptr;
        }

        [[nodiscard]] virtual ASTNode *nud(Parser *parser) {
            assert(false);
            return nullptr;
        }

        virtual void add(ASTNode *c) {
            _children.emplace_back(c);
        }

        void printTree() {
            using std::cout;
            std::cout << ast_type_names[this->_op] << "\n";
            printSubtree("");
            cout << "\n";
        }

        void printSubtree(const std::string &prefix) {
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
    };

    class ASTInfixBinaryOp : public ASTNode {
    public:
        ASTInfixBinaryOp() : ASTNode(ASTType::EOF_, 0, op_precedence[ASTType::EOF_], 0) {
        };

        [[nodiscard]] ASTNode *led(ASTNode *left, Parser *parser) override {
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

        [[nodiscard]] ASTNode *nud(Parser *parser) override {
            assert(false);
            return nullptr;
        }
    };

    class ASTNumberLiteral final : public ASTNode {
    public:
        ASTNumberLiteral(const std::string &str, bool is_float) : ASTNode(ASTType::NUM_LITERAL, 1,
                                                                          op_precedence[ASTType::NUM_LITERAL], 0) {
            _is_float = is_float;
            if (is_float) {
                _fvalue = std::stof(str);
            } else {
                _ivalue = std::stoi(str);
            }
        };

        [[nodiscard]] ASTNode *led(ASTNode *left, Parser *parser) override {
            assert(false);
            return nullptr;
        }

        [[nodiscard]] ASTNode *nud(Parser *parser) override {
            return this;
        }

        [[nodiscard]] bool is_float() const {
            return _is_float;
        }

        [[nodiscard]] int get_ivalue() const override {
            assert(!_is_float);
            return _ivalue;
        }

        [[nodiscard]] float get_fvalue() const override {
            assert(_is_float);
            return _fvalue;
        }

    private:
        bool _is_float = false;
        union {
            int _ivalue;
            float _fvalue;
        };
    };

    class ASTStringLiteral final : public ASTNode {
    public:
        ASTStringLiteral(std::string str) : ASTNode(ASTType::STRING_LITERAL, 1, op_precedence[ASTType::STRING_LITERAL],
                                                    0) {
            _svalue = std::move(str);
        }

        std::string get_svalue() const override {
            return _svalue;
        }

    private:
        std::string _svalue;
    };

    class ASTSum final : public ASTInfixBinaryOp {
    public:
        ASTSum() : ASTInfixBinaryOp() {
            _op = ASTType::SUM;
            _lbp = op_precedence[ASTType::SUM];
        };
    };

    class ASTSubtract final : public ASTInfixBinaryOp {
    public:
        ASTSubtract() : ASTInfixBinaryOp() {
            _op = ASTType::SUBTRACT;
            _lbp = op_precedence[ASTType::SUBTRACT];
        };
    };

    class ASTMultiply final : public ASTInfixBinaryOp {
    public:
        ASTMultiply() : ASTInfixBinaryOp() {
            _op = ASTType::MULTIPLY;
            _lbp = op_precedence[ASTType::MULTIPLY];
        };
    };

    class ASTDivide final : public ASTInfixBinaryOp {
    public:
        ASTDivide() : ASTInfixBinaryOp() {
            _op = ASTType::DIVIDE;
            _lbp = op_precedence[ASTType::DIVIDE];
        };
    };
}

#endif /* TAN_PARSER_H */
