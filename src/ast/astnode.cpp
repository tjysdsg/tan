#include "src/ast/astnode.h"
#include <iostream>

namespace tanlang {

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

ASTNode::~ASTNode() {
    for (auto *&c : _children) {
        delete c;
        c = nullptr;
    }
}
ASTNode *ASTNode::led(ASTNode *left, Parser *parser) {
    UNUSED(left);
    UNUSED(parser);
    assert(false);
    return nullptr;
}
ASTNode *ASTNode::nud(Parser *parser) {
    UNUSED(parser);
    assert(false);
    return nullptr;
}
void ASTNode::add(ASTNode *c) {
    _children.emplace_back(c);
}
int ASTNode::get_ivalue() const {
    assert(false);
    return 0;
}
float ASTNode::get_fvalue() const {
    assert(false);
    return 0;
}
std::string ASTNode::get_svalue() const {
    assert(false);
    return "";
}
ASTNode::ASTNode(ASTType op, int associativity, int lbp, int rbp)
    : _op(op), _associativity(associativity), _lbp(lbp), _rbp(rbp) {}

ASTNode *ASTInfixBinaryOp::led(ASTNode *left, Parser *parser) {
    _children.emplace_back(left);
    auto *n = parser->next_expression(_lbp);
    if (!n) {
        // TODO: report error
        throw "SHIT";
    } else {
        _children.emplace_back(n);
    }
    return this;
}

ASTNode *ASTInfixBinaryOp::nud(Parser *parser) {
    UNUSED(parser);
    assert(false);
    return nullptr;
}
ASTInfixBinaryOp::ASTInfixBinaryOp() : ASTNode(ASTType::EOF_, 0, op_precedence[ASTType::EOF_], 0) {
}

ASTNumberLiteral::ASTNumberLiteral(const std::string &str, bool is_float) : ASTNode(ASTType::NUM_LITERAL,
                                                                                    1,
                                                                                    op_precedence[ASTType::NUM_LITERAL],
                                                                                    0) {
    _is_float = is_float;
    if (is_float) {
        _fvalue = std::stof(str);
    } else {
        _ivalue = std::stoi(str);
    }
}
ASTNode *ASTNumberLiteral::nud(Parser *parser) {
    UNUSED(parser);
    return this;
}
bool ASTNumberLiteral::is_float() const {
    return _is_float;
}
int ASTNumberLiteral::get_ivalue() const {
    assert(!_is_float);
    return _ivalue;
}
float ASTNumberLiteral::get_fvalue() const {
    assert(_is_float);
    return _fvalue;
}

ASTNode *ASTPrefix::nud(Parser *parser) {
    auto *n = parser->next_expression(_lbp);
    if (!n) {
        // TODO: report error
    } else {
        _children.emplace_back(n);
    }
    return this;
}
ASTPrefix::ASTPrefix() : ASTNode(ASTType::EOF_, 1, op_precedence[ASTType::EOF_], 0) {}
ASTStringLiteral::ASTStringLiteral(std::string str) : ASTNode(ASTType::STRING_LITERAL, 1,
                                                              op_precedence[ASTType::STRING_LITERAL],
                                                              0), _svalue(std::move(str)) {
}
std::string ASTStringLiteral::get_svalue() const {
    return _svalue;
}
ASTLogicalNot::ASTLogicalNot() : ASTPrefix() {
    _op = ASTType::LNOT;
    _lbp = op_precedence[_op];
}
ASTBinaryNot::ASTBinaryNot() : ASTPrefix() {
    _op = ASTType::BNOT;
    _lbp = op_precedence[_op];
}
ASTMultiply::ASTMultiply() : ASTInfixBinaryOp() {
    _op = ASTType::MULTIPLY;
    _lbp = op_precedence[ASTType::MULTIPLY];
}
ASTDivide::ASTDivide() : ASTInfixBinaryOp() {
    _op = ASTType::DIVIDE;
    _lbp = op_precedence[ASTType::DIVIDE];
}
ASTSum::ASTSum() : ASTInfixBinaryOp() {
    _op = ASTType::SUM;
    _lbp = op_precedence[ASTType::SUM];
}
ASTSubtract::ASTSubtract() : ASTInfixBinaryOp() {
    _op = ASTType::SUBTRACT;
    _lbp = op_precedence[ASTType::SUBTRACT];
}

#define MAKE_ASTTYPE_NAME_PAIR(t) {ASTType::t, #t}

std::unordered_map<ASTType, std::string> ast_type_names{
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
std::unordered_map<ASTType, int> op_precedence{
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

    {ASTType::NUM_LITERAL, PREC_LITERAL},
    {ASTType::STRING_LITERAL, PREC_LITERAL}
};
} // namespace tanlang
