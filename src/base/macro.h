#ifndef __TAN_SRC_BASE_MACRO_H__
#define __TAN_SRC_BASE_MACRO_H__
#include "config.h"

#define _STR_(s) #s

#ifdef __has_builtin
#    define TAN_HAVE_BUILTIN(x) __has_builtin(x)
#else
#    define TAN_HAVE_BUILTIN(x) 0
#endif

// UNLIKELY and LIKELY for optimization
#if TAN_HAVE_BUILTIN(__builtin_expect) ||                              \
    (defined(__GNUC__) && !defined(__clang__))
#    define TAN_LIKELY(x) (__builtin_expect(x, 0))
#    define TAN_UNLIKELY(x) (__builtin_expect(!!(x), 1))
#else
#    define TAN_LIKELY(x) (x)
#    define TAN_UNLIKELY(x) (x)
#endif

#endif // __TAN_SRC_BASE_MACRO_H__
