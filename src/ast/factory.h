#ifndef __TAN_SRC_AST_FACTORY_H__
#define __TAN_SRC_AST_FACTORY_H__
#include "src/ast/astnode.h"
#include "src/ast/ast_identifier.h"

namespace tanlang {

template<typename T> class ASTFactory {
public:
  static std::shared_ptr<T> Create() {
    throw std::runtime_error("NOT IMPLEMENTED");
  }
};

template<> class ASTFactory<ASTIdentifier> {
  using AST_T = ASTIdentifier;

public:
  static std::shared_ptr<ASTIdentifier> Create(std::string name) {
    auto ret = std::make_shared<AST_T>(name, nullptr);
    return ret;
  }
};

template<> class ASTFactory<ASTVarDecl> {
  using AST_T = ASTVarDecl;

public:
  static std::shared_ptr<AST_T> Create(std::string name, std::shared_ptr<ASTTy> ty) {
    auto ret = std::make_shared<AST_T>();
    ret->_children.push_back(ASTFactory<ASTIdentifier>::Create(name));
    ret->_children.push_back(ty);
    return ret;
  }
};

}

#endif /* __TAN_SRC_AST_FACTORY_H__ */
