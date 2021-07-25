#include "base.h"
#include "reader.h"
#include "token.h"
#include <fmt/core.h>
#include <iostream>
#include <backtrace.h>

#ifdef _MSC_VER
#include <windows.h>
#include <intrin.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

struct StackFrame {
  DWORD64 address;
  str name;
  unsigned int line;
  str file;
};

inline vector<StackFrame> stack_trace() {
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
    return vector<StackFrame>();
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

  vector<StackFrame> frames;
  while (StackWalk(machine,
      process,
      thread,
      &frame,
      &context,
      nullptr,
      SymFunctionTableAccess,
      SymGetModuleBase,
      nullptr)) {
    StackFrame f = {};
    f.address = frame.AddrPC.Offset;

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
    char symbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
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

static void print_back_trace() {
  vector<StackFrame> stack = stack_trace();
  for (auto &i : stack) {
    std::cerr << "Callstack:\n0x" << std::hex << i.address << ": " << i.name << "(" << i.line << ")\n";
  }
}
#else
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
  std::cout << fmt::format("{}:{} in function {}\n", filename, lineno, func_name);
  return 0;
}

void bt_error_callback(void *, const char *msg, int errnum) {
  std::cerr << fmt::format("Error {} occurred when getting the stacktrace: {}", errnum, msg);
}

void bt_error_callback_create(void *, const char *msg, int errnum) {
  std::cerr << fmt::format("Error {} occurred when initializing the stacktrace: {}", errnum, msg);
}

void init_back_trace(const char *filename) {
  __bt_state = backtrace_create_state(filename, 0, bt_error_callback_create, nullptr);
}

void print_back_trace() {
  TAN_ASSERT(__bt_state); /// make sure init_back_trace() is called
  backtrace_full((backtrace_state *) __bt_state, 0, bt_callback, bt_error_callback, nullptr);
}

#endif

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  std::cerr << "ASSERTION FAILED: " << expr << "\n";
  std::cerr << "at: " << file << ":" << std::to_string(lineno) << "\n";
  print_back_trace();
  abort();
}

[[noreturn]] void __tan_assert_fail() {
  print_back_trace();
  abort();
}

#ifdef DEBUG
#define ABORT() __tan_assert_fail()
#else
#define ABORT() exit(1)
#endif

namespace tanlang {

void report_error(const str &error_message) {
  std::cerr << "[ERROR] " << error_message << "\n";
  ABORT();
}

void report_error(const str &filename, const str &source, size_t line, size_t col, const str &error_message) {
  str indent = col > 0 ? str(col - 1, ' ') : "";
  std::cerr << fmt::format("[ERROR] at {}:{} {}\n{}\n{}^\n", filename, line, error_message, source, indent);
  ABORT();
}

void report_error(const str &filename, Token *token, const str &error_message) {
  str indent = token->get_col() > 0 ? str(token->get_col() - 1, ' ') : "";
  std::cerr << fmt::format("[ERROR] at {}:{} {}\n{}\n{}^\n",
      filename,
      token->get_line() + 1,
      error_message,
      token->get_source_line(),
      indent);
  ABORT();
}

} // namespace tanlang
