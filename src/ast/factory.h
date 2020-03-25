#ifndef __TAN_SRC_AST_FACTORY_H__
#define __TAN_SRC_AST_FACTORY_H__
#include "src/ast/ast_identifier.h"
#include "src/ast/astnode.h"

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

  static std::shared_ptr<AST_T> Create(std::string name, std::shared_ptr<ASTTy> ty, std::shared_ptr<ASTLiteral> value) {
    auto ret = ASTFactory<AST_T>::Create(name, ty);
    ret->_children.push_back(value);
    ret->_has_initial_val = true;
    return ret;
  }
};

template<> class ASTFactory<ASTNumberLiteral> {
  using AST_T = ASTNumberLiteral;

public:
  static std::shared_ptr<AST_T> Create(float value) {
    auto ret = std::make_shared<AST_T>(value);
    return ret;
  }

  static std::shared_ptr<AST_T> Create(int value) {
    auto ret = std::make_shared<AST_T>(value);
    return ret;
  }
};

} // namespace tanlang

#endif /* __TAN_SRC_AST_FACTORY_H__ */
