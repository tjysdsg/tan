#ifndef __TAN_SRC_AST_VALUED_AST_NODE_H__
#define __TAN_SRC_AST_VALUED_AST_NODE_H__

namespace tanlang {

class ValuedASTNode {
  virtual void set_value(str str_value) = 0;
  virtual void set_value(uint64_t int_value) = 0;
  virtual void set_value(double float_value) = 0;
  virtual uint64_t get_int_value() = 0;
  virtual str get_str_value() = 0;
  virtual double get_float_value() = 0;
};

}

#endif //__TAN_SRC_AST_VALUED_AST_NODE_H__
