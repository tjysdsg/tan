#ifndef __TAN_SRC_AST_INTERFACE_H__
#define __TAN_SRC_AST_INTERFACE_H__
#include <string>

namespace llvm {
class Type;

class Value;
}

namespace tanlang {

// virtual std::string get_name() const = 0;
// virtual std::string get_type_name() const = 0;
// virtual llvm::Type *to_llvm_type(CompilerSession *) const = 0;
// virtual llvm::Value *get_llvm_value(CompilerSession *) const = 0;
// virtual bool is_lvalue() const = 0;

} // namespace tanlang

#endif /* __TAN_SRC_AST_INTERFACE_H__ */
