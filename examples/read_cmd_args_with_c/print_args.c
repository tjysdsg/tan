#include <stdio.h>
#include "print_args.h"

void print_args(int argc, char **argv) {
  printf("Number of arguments %d\n", argc);
  for (int i = 0; i < argc; ++i) {
    printf("Argument #%d: %s\n", i + 1, argv[i]);
  }
}


