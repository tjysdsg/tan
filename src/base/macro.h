#ifndef __TAN_SRC_BASE_MACRO_H__
#define __TAN_SRC_BASE_MACRO_H__

#include "config.h"

/// \brief for suppressing unused parameters warning in a function
#define UNUSED(x) (void)(x)

#define __s__(x) #x
#define __STR__(x) __s__(x)

#ifdef __has_builtin
#    define TAN_HAVE_BUILTIN(x) __has_builtin(x)
#else
#    define TAN_HAVE_BUILTIN(x) 0
#endif

// UNLIKELY and LIKELY for optimization
#if TAN_HAVE_BUILTIN(__builtin_expect) || (defined(__GNUC__) && !defined(__clang__))
#   define TAN_LIKELY(x) (__builtin_expect(x, 0))
#   define TAN_UNLIKELY(x) (__builtin_expect(!!(x), 1))
#else
#   define TAN_LIKELY(x) (x)
#   define TAN_UNLIKELY(x) (x)
#endif

#define p_cast(T, val) reinterpret_cast<T>(val)
#define c_cast(T, val) const_cast<T>(val)

#ifdef MSVC
#define CALL_CONV_C __cdecl
#define CALL_CONV_CPP __stdcall
#else
#define CALL_CONV_C __attribute__((cdecl))
#define CALL_CONV_CPP __attribute__((stdcall))
#endif

#endif // __TAN_SRC_BASE_MACRO_H__
