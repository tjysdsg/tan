#include "ast/package.h"

using namespace tanlang;

Package::Package(const str &name, vector<ASTBase *> subtrees)
    : ASTBase(ASTNodeType::PROGRAM, nullptr, 0), _name(name), _subtrees(subtrees) {}

vector<ASTBase *> Package::get_children() const { return _subtrees; }
