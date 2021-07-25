extern "C" void init_back_trace(const char *filename);
extern "C" int tan_main(int argc, char **argv);

int main(int argc, char **argv) {
  init_back_trace(argv[0]);
  return tan_main(argc, argv);
}
