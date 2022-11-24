#ifndef __TAN_SRC_AST_CONSTRUCTOR_H__
#define __TAN_SRC_AST_CONSTRUCTOR_H__
#include "base.h"
#include "src/ast/fwd.h"
#include "src/ast/source_manager.h"
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
  explicit Constructor(ConstructorType type) : _type(type) {};

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
  static BasicConstructor *CreateIntegerConstructor(SrcLoc loc,
      uint64_t default_val = 0,
      size_t bit_size = 32,
      bool is_unsigned = false);
  static BasicConstructor *CreateBoolConstructor(SrcLoc loc, bool default_val = false);
  static BasicConstructor *CreateFPConstructor(SrcLoc loc, double default_val = 0, size_t bit_size = 32);
  static BasicConstructor *CreateStringConstructor(SrcLoc loc, str default_val = "");
  static BasicConstructor *CreateCharConstructor(SrcLoc loc, uint8_t default_val = 0);
  static BasicConstructor *CreateArrayConstructor(SrcLoc loc, Type *element_type = {});
  static BasicConstructor *CreateNullPointerConstructor(SrcLoc loc, Type *element_type);

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

}

#endif //__TAN_SRC_AST_CONSTRUCTOR_H__
