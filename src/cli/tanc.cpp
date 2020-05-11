#include "cli.h"
#include "backtrace.h"
#include <csignal>

static backtrace_state *g_bt_state = nullptr;

void sig_handler(int) { backtrace_print(g_bt_state, 0, stdout); }

int main(int argc, char **argv) {
  /// setup backtrace
  g_bt_state = backtrace_create_state(nullptr, 0, nullptr, nullptr);
  signal(SIGSEGV, sig_handler);
  signal(SIGILL, sig_handler);
  signal(SIGABRT, sig_handler);
  return cli_main(&argc, &argv);
}
