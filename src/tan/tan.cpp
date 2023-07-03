#include "tan/tan.h"
#include "backtrace/tan_backtrace.h"

namespace tanlang {

bool init_compiler(int, char **argv) { return init_back_trace(argv[0]); }

} // namespace tanlang
