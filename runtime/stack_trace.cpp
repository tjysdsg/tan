#include "stack_trace.h"
#include <malloc.h>
#include <execinfo.h>

extern "C" void tan_print_st() {
  void *buffer = malloc(2048);
  int st_size = backtrace((void **) buffer, 256);
  char **bt = backtrace_symbols((void *const *) buffer, st_size);
  for (int i = 0; i < st_size; ++i) {
    printf("%s\n", bt[i]);
  }
}
