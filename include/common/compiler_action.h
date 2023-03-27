#ifndef __TAN_COMMON_COMPILER_ACTION_H__
#define __TAN_COMMON_COMPILER_ACTION_H__

#include "common/ast_visitor.h"

namespace tanlang {

template <typename C, typename Input, typename Output>
concept HasImpl = requires(C c, Input input) {
                    { c.run_impl(input) } -> std::same_as<Output>;
                  };

template <typename Derived, typename Input, typename Output> class CompilerAction : public ASTVisitor<Derived> {
public:
  using CompilerActionType = CompilerAction<Derived, Input, Output>;

  virtual ~CompilerAction() = default;

  Output run(Input input) {
    static_assert(HasImpl<Derived, Input, Output>);

    init(input);
    return ((Derived *)this)->run_impl(input);
  }

protected:
  virtual void init(Input) {}
};

} // namespace tanlang

#endif // __TAN_COMMON_COMPILER_ACTION_H__
