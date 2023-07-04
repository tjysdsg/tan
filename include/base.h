#ifndef __TAN_BASE_H__
#define __TAN_BASE_H__

/**
 * \file Include bunch of basic utilities
 * */

#include <fmt/core.h>
#include "base/error.h"
#include "base/macro.h"
#include "base/container.h"
#include "base/utils.h"

#ifdef _MSC_VER
// fix bunch of errors caused by macros defined in windows.h
#define NOMINMAX
#include <windows.h>
#include <DbgHelp.h>
#include <llvm/Support/Windows/WindowsSupport.h>
#undef min
#undef max
#undef OPTIONAL
#undef CONST
#undef VOID
#endif

#endif
