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
#define MAX_STACK_TRACE 256

void *__bt_state = nullptr;

struct frame_info {
  char *filename;
  int lineno;
  char *function;
};

struct bt_data {
  struct frame_info *all;
  size_t index;
  size_t max;
  int failed;
};

/*
static void shit() {
  unw_cursor_t cursor;
  unw_context_t context;
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    std::printf("0x%lx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char *nameptr = sym;
      int status;
      char *demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      std::printf(" (%s+0x%lx)\n", nameptr, offset);
      std::free(demangled);
    } else {
      std::printf("unable to obtain symbol name for this frame\n");
    }
  }
}
*/

int bt_callback(void *vdata, uintptr_t pc, const char *filename, int lineno, const char *function) {
  auto *data = (struct bt_data *) vdata;

  if (data->index >= data->max) {
    fprintf(stderr, "Number of stack frames exceeds %d\n", MAX_STACK_TRACE);
    data->failed = 1;
    return 1;
  }

  frame_info *p = &data->all[data->index];
  if (filename == nullptr) {
    p->filename = nullptr;
  } else {
    p->filename = strdup(filename);
  }
  p->lineno = lineno;
  if (function == nullptr) {
    p->function = nullptr;
  } else {
    p->function = strdup(function);
  }
  ++data->index;

  std::cout << fmt::format("{}:{} in function {}\n", filename, lineno, function);
  return 0;
}

void bt_error_callback(void *vdata, const char *msg, int errnum) {
  auto *data = (struct bt_data *) vdata;
  fprintf(stderr, "%s", msg);
  if (errnum > 0) {
    fprintf(stderr, ": %s", strerror(errnum));
  }
  fprintf(stderr, "\n");
  data->failed = 1;
}

void bt_error_callback_create(void *data, const char *msg, int errnum) {
  fprintf(stderr, "%s", msg);
  if (errnum > 0) {
    fprintf(stderr, ": %s", strerror(errnum));
  }
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}

void init_back_trace(const char *filename) {
  __bt_state = backtrace_create_state(filename, 0, bt_error_callback_create, nullptr);
}

void print_back_trace() {
  TAN_ASSERT(__bt_state); /// make sure init_back_trace() is called
  frame_info all[MAX_STACK_TRACE];
  bt_data data;

  data.all = &all[0];
  data.index = 0;
  data.max = MAX_STACK_TRACE;
  data.failed = 0;

  backtrace_full((backtrace_state *) __bt_state, 0, bt_callback, bt_error_callback, &data);
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
