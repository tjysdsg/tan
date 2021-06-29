#ifndef __TAN_SRC_AST_CONSTRUCTOR_H__
#define __TAN_SRC_AST_CONSTRUCTOR_H__
#include "base.h"
#include "src/ast/fwd.h"
#include <variant>

namespace llvm { class Value; }

namespace tanlang {

enum class ConstructorType {
  BASIC, STRUCT,
};

class Constructor {
public:
  Constructor() = delete;

protected:
  Constructor(ConstructorType type) : _type(type) {};

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
  static BasicConstructor *CreateIntegerConstructor(CompilerSession *cs,
      uint64_t default_val = 0,
      bool is_unsigned = false);
  static BasicConstructor *CreateFPConstructor(CompilerSession *cs, double default_val = 0);
  static BasicConstructor *CreateStringConstructor(CompilerSession *cs, str default_val = "");
  static BasicConstructor *CreateCharConstructor(CompilerSession *cs, uint8_t default_val = 0);
  static BasicConstructor *CreateArrayConstructor(CompilerSession *cs, vector<Literal *> default_val = {});

  CompTimeExpr *get_value() const;
  void set_value(CompTimeExpr *val);

protected:
  BasicConstructor() : Constructor(ConstructorType::BASIC) {};

private:
  CompTimeExpr *_value = nullptr;
};

// TODO: support user-defined constructor
class StructConstructor : public Constructor {
public:
  StructConstructor() = delete;
  static StructConstructor *Create(ASTType *struct_type);
  static StructConstructor *Create(ASTType *struct_type, vector<Constructor *> member_ctrs);

  vector<Constructor *> &get_member_constructors();
  void set_member_constructors(const vector<Constructor *> &member_constructors);
  ASTType *get_struct_type() const;

protected:
  StructConstructor(ASTType *struct_type);

private:
  vector<Constructor *> _member_constructors{};
  ASTType *_struct_type = nullptr;
};

}

#endif //__TAN_SRC_AST_CONSTRUCTOR_H__
