#include "base.h"
#include "reader.h"
#include "token.h"
#include "compiler_session.h"
#include <iostream>

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
#define UNW_LOCAL_ONLY
#include <libunwind.h>
#include <cxxabi.h>

static void print_back_trace() {
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
#endif

[[noreturn]] void __tan_assert_fail(const char *expr, const char *file, size_t lineno) {
  std::cerr << "ASSERTION FAILED: " << expr << "\n";
  std::cerr << "at: " << file << ":" << std::to_string(lineno) << "\n";
  print_back_trace();
  abort();
}

namespace tanlang {

void report_error(const str &error_message) {
  std::cerr << "[ERROR] " << error_message << "\n";
  exit(1);
}

void report_error(const str &source, size_t line, size_t col, const str &error_message) {
  str indent = col > 0 ? str(col - 1, ' ') : "";
  std::cerr << "[ERROR] at LINE" << std::to_string(line) << " " << error_message << "\n" << source << "\n" << indent
            << "^\n";
  exit(1);
}

void report_error(const str &filename, Token *token, const str &error_message) {
  str indent = token->c > 0 ? str(token->c - 1, ' ') : "";
  std::cerr << "[ERROR] at " << filename << ":" << std::to_string(token->l + 1) << " " << error_message << "\n"
            << token->line->code << "\n" << indent << "^\n";
  exit(1);
}

void error(CompilerSession *cs, const str &error_message) {
  if (cs && cs->_current_token) {
    report_error(cs->_filename, cs->_current_token, error_message);
  } else { report_error(error_message); }
}

} // namespace tanlang
