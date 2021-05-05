#ifndef __TAN_SRC_AST_PARSABLE_AST_NODE_H__
#define __TAN_SRC_AST_PARSABLE_AST_NODE_H__
#include "base.h"
#include "src/ast/source_traceable.h"
#include "src/ast/precedence.h"
#include "src/ast/ast_type.h"

namespace tanlang {

class ParsableASTNode : public SourceTraceable {

public:
  virtual ~ParsableASTNode() = 0;

public:
  virtual ptr<ParsableASTNode> get_child_at(size_t idx) = 0;
  virtual void set_child_at(size_t idx, ptr<ParsableASTNode> node) = 0;
  virtual void append_child(ptr<ParsableASTNode> node) = 0;
  virtual ASTType get_node_type() = 0;
  virtual void set_node_type(ASTType node_type) = 0;
  virtual void set_lbp(int _lbp) = 0;
  virtual int get_lbp() = 0;
  virtual void set_value(str str_value) = 0;
  virtual void set_value(uint64_t int_value) = 0;
  virtual void set_value(double float_value) = 0;
  virtual uint64_t get_int_value() = 0;
  virtual str get_str_value() = 0;
  virtual double get_float_value() = 0;

  // bool _parsed = false;
  // bool _is_typed = false;
  // bool _is_valued = false;
  // bool _is_named = false;
};

}

#endif //__TAN_SRC_AST_PARSABLE_AST_NODE_H__
