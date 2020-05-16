#ifndef __TAN_SRC_LIB_MISC_H__
#define __TAN_SRC_LIB_MISC_H__
#include "base.h"
#include "libtanc.h"

inline str opt_level_to_string(TanOptLevel l) {
  switch (l) {
    case O0:
      return "-O0";
    case O1:
      return "-O1";
    case O2:
      return "-O2";
    case O3:
      return "-O3";
    default:
      TAN_ASSERT(false);
  }
}

#endif //__TAN_SRC_LIB_MISC_H__
