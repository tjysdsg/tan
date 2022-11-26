#include "stack_trace.h"

extern "C" void print_back_trace(); // in src/backtrace/tan_backtrace.cpp

extern "C" void __tan_runtime_stack_trace() { print_back_trace(); }
