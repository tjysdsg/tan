#ifndef __TAN_SRC_AST_CONSTRUCTOR_H__
#define __TAN_SRC_AST_CONSTRUCTOR_H__
#include "base.h"
#include "fwd.h"
#include "source_file/source_manager.h"
#include <variant>

namespace llvm {
class Value;
}

namespace tanlang {

enum class ConstructorType {
  BASIC,
  STRUCT,
};

class Constructor {
public:
  Constructor() = delete;

protected:
  explicit Constructor(ConstructorType type) : _type(type){};

public:
  virtual ~Constructor() {}

  ConstructorType get_type() const { return _type; }

  void set_type(ConstructorType type) { _type = type; }

private:
  ConstructorType _type = ConstructorType::BASIC;
};

class BasicConstructor : public Constructor {
public:
  /**
   * \note Make sure default_val's type is resolved
   */
  static BasicConstructor *Create(CompTimeExpr *default_val);

public:
  CompTimeExpr *get_value() const;

protected:
  BasicConstructor() : Constructor(ConstructorType::BASIC){};

private:
  CompTimeExpr *_value = nullptr;
};

// TODO: support user-defined constructor
class StructConstructor : public Constructor {
public:
  StructConstructor() = delete;
  static StructConstructor *Create(Type *struct_type);
  static StructConstructor *Create(Type *struct_type, vector<Constructor *> member_ctrs);

  vector<Constructor *> &get_member_constructors();
  void set_member_constructors(const vector<Constructor *> &member_constructors);
  Type *get_struct_type() const;

protected:
  explicit StructConstructor(Type *struct_type);

private:
  vector<Constructor *> _member_constructors{};
  Type *_struct_type = nullptr;
};

} // namespace tanlang

#endif //__TAN_SRC_AST_CONSTRUCTOR_H__
