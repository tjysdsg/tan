#include "stack_trace.h"
#include <malloc.h>

extern "C" void stack_trace() {
  /* TODO
      unw_cursor_t cursor;
      unw_context_t context;
      /// initialize cursor to current frame for local unwinding.
      unw_getcontext(&context);
      unw_init_local(&cursor, &context);
      /// unwind frames one by one, going up the frame stack.
      while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
          break;
        }
        printf("0x%lx:", pc);
        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
          printf(" (%s+0x%lx)\n", sym, offset);
        } else {
          printf("unable to obtain symbol name for this frame\n");
        }
      }
   */
}
