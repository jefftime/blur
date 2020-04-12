#include "error.h"
#include "window.h"             /* window_new, window_del */

int main(int argc, char **argv) {
  enum {
    width = 640,
    height = 480
  };

  struct window *w;

  w = window_new("Test", width, height);
  for (;;) {
    if (window_should_close(w)) break;
    window_update(w);
  }
  window_del(w);
  return 0;
}
