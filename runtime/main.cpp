#include <cstdio>
#include <cstdlib>

extern "C" bool init_back_trace(const char *filename);
extern "C" int tan_main(int argc, char **argv);

int main(int argc, char **argv) {
  if (!init_back_trace(argv[0])) {
    printf("Unable to init tan runtime\n");
    abort();
  }
  return tan_main(argc, argv);
}
