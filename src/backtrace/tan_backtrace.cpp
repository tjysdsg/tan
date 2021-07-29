#include "tan_backtrace.h"
#include <cstdlib>

#ifdef _MSC_VER
#include <windows.h>
#include <intrin.h>
#include <dbghelp.h>
#include <string>
#include <vector>
#include <iostream>
#pragma comment(lib, "dbghelp.lib")

struct StackFrame {
  std::string name;
  unsigned int line;
  std::string file;
};

void init_back_trace(const char *) {}

inline std::vector<StackFrame> stack_trace() {
  using std::cerr;
  #if _WIN64
  DWORD machine = IMAGE_FILE_MACHINE_AMD64;
  #else
  DWORD machine = IMAGE_FILE_MACHINE_I386;
  #endif
  HANDLE process = GetCurrentProcess();
  HANDLE thread = GetCurrentThread();

  if (SymInitialize(process, nullptr, TRUE) == FALSE) {
    cerr << "Failed to call SymInitialize\n";
    return std::vector<StackFrame>();
  }

  SymSetOptions(SYMOPT_LOAD_LINES);

  CONTEXT context = {};
  context.ContextFlags = CONTEXT_FULL;
  RtlCaptureContext(&context);

  #if _WIN64
  STACKFRAME frame = {};
  frame.AddrPC.Offset = context.Rip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Rbp;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Rsp;
  frame.AddrStack.Mode = AddrModeFlat;
  #else
  STACKFRAME frame = {};
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  #endif

  bool first = true;

  std::vector<StackFrame> frames;
  while (StackWalk(machine,
      process,
      thread,
      &frame,
      &context,
      nullptr,
      SymFunctionTableAccess,
      SymGetModuleBase,
      nullptr)) {
    StackFrame f{};

    #if _WIN64
    DWORD64 moduleBase = 0;
    #else
    DWORD moduleBase = 0;
    #endif

    moduleBase = SymGetModuleBase(process, frame.AddrPC.Offset);

    #if _WIN64
    DWORD64 offset = 0;
    #else
    DWORD offset = 0;
    #endif
    char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255]{0};
    auto symbol = (PIMAGEHLP_SYMBOL) symbolBuffer;
    symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL) + 255;
    symbol->MaxNameLength = 254;

    if (SymGetSymFromAddr(process, frame.AddrPC.Offset, &offset, symbol)) {
      f.name = symbol->Name;
    } else {
      DWORD error = GetLastError();
      printf("Failed to resolve address 0x%llX: %lu\n", frame.AddrPC.Offset, error);
      f.name = "Unknown Function";
    }

    IMAGEHLP_LINE line;
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

    DWORD offset_ln = 0;
    if (SymGetLineFromAddr(process, frame.AddrPC.Offset, &offset_ln, &line)) {
      f.file = line.FileName;
      f.line = line.LineNumber;
    } else {
      DWORD error = GetLastError();
      printf("Failed to resolve line for 0x%llX%lu\n", frame.AddrPC.Offset, error);
      f.line = 0;
    }

    if (!first) {
      frames.push_back(f);
    }
    first = false;
  }

  SymCleanup(process);
  return frames;
}

void print_back_trace() {
  std::vector<StackFrame> stack = stack_trace();
  for (auto &i : stack) {
    printf("%s:%d in function %s\n", i.file.c_str(), i.line, i.name.c_str());
  }
}
#else
#include <backtrace.h>
#include <cxxabi.h>

void *__bt_state = nullptr;

int bt_callback(void *, uintptr_t, const char *filename, int lineno, const char *function) {
  /// demangle function name
  const char *func_name = function;
  int status;
  char *demangled = abi::__cxa_demangle(function, nullptr, nullptr, &status);
  if (status == 0) {
    func_name = demangled;
  }

  /// print
  printf("%s:%d in function %s\n", filename, lineno, func_name);
  return 0;
}

void bt_error_callback(void *, const char *msg, int errnum) {
  printf("Error %d occurred when getting the stacktrace: %s", errnum, msg);
}

void bt_error_callback_create(void *, const char *msg, int errnum) {
  printf("Error %d occurred when initializing the stacktrace: %s", errnum, msg);
}

void init_back_trace(const char *filename) {
  __bt_state = backtrace_create_state(filename, 0, bt_error_callback_create, nullptr);
}

void print_back_trace() {
  if (!__bt_state) { /// make sure init_back_trace() is called
    printf("Make sure init_back_trace() is called before calling print_stack_trace()\n");
    abort();
  }
  backtrace_full((backtrace_state *) __bt_state, 0, bt_callback, bt_error_callback, nullptr);
}

#endif
