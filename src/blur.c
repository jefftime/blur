#include "error.h"
#include "window.h"             /* window_new, window_del */
#include "keypoll.h"            /* kp_new, kp_del */

int main(int argc, char **argv) {
  enum {
    width = 640,
    height = 480
  };

  struct window *w = NULL;
  struct kp_ctx *kp = NULL;

  w = window_new("Test", width, height);
  if (!w) goto err;
  kp = kp_new();
  if (!kp) goto err;
  for (;;) {
    if (window_should_close(w)) break;
    kp_update(kp);
    window_update(w);
  }
  window_del(w);
  kp_del(kp);
  return 0;

 err:
  window_del(w);
  kp_del(kp);
  return -1;
}
