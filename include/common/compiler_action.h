#ifndef __TAN_COMMON_COMPILER_ACTION_H__
#define __TAN_COMMON_COMPILER_ACTION_H__

#include "common/ast_visitor.h"

namespace tanlang {

template <class T, class U>
concept SameHelper = std::is_same_v<T, U>;

/**
 * \brief Some compilers don't have std::same_as
 */
template <class T, class U>
concept same_as = SameHelper<T, U> && SameHelper<U, T>;

template <typename C, typename Input, typename Output>
concept HasImpl = requires(C c, Input input) {
                    { c.run_impl(input) } -> same_as<Output>;
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
