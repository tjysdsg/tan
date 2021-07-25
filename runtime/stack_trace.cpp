#include <cstdio>

extern "C" void print_back_trace();

extern "C" void stack_trace() {
  print_back_trace();
}
