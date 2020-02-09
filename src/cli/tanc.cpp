#include "src/cli/App.h"
#include "parser.h"

int main(int argc, char **argv) {
  App<tanlang::Parser> app(argc, argv);
  bool r;
  while (true) {
    r = app.read();
    if (!r) break;
    r &= app.parse();
    if (!r) break;
    r &= app.compile();
    if (!r) break;
    app.next_file();
  }
  return 0;
}
