#ifndef __TAN_SRC_LIB_LLVM_AR_H__
#define __TAN_SRC_LIB_LLVM_AR_H__
#include "base.h"

/**
 * \brief Combines several object files into an archive file (*.a).
 * Exit program with status code 1 if any error is encountered.
 */
extern void llvm_ar_create_static_lib(const str &archive_name, const vector<str> &objects);

#endif //__TAN_SRC_LIB_LLVM_AR_H__
