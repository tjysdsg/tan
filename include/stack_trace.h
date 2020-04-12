#ifndef __TAN_INCLUDE_STACK_TRACE_H__
#define __TAN_INCLUDE_STACK_TRACE_H__
#include <string>

namespace tanlang {

struct StackTrace {
  std::string _filename = "";
  std::string _src = "";
  size_t _lineno = 0;
};

}

#endif /* __TAN_INCLUDE_STACK_TRACE_H__ */
