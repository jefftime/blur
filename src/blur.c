#include "error.h"
#include "sized_types.h"
#include "window.h"
#include "keypoll.h"
#include "render.h"
#include "xrand.h"
#include <time.h>

int main(int argc, char **argv) {
  enum {
    width = 640,
    height = 480,
    render_width = 320,
    render_height = 240
  };

  struct window *w = NULL;
  struct kp_ctx *kp = NULL;
  struct render *r = NULL;

  XRAND_SEED = (uint64_t) time(NULL);
  w = window_new("Test", width, height);
  if (!w) goto err;
  kp = kp_new();
  if (!kp) goto err;
  r = render_new(w);
  if (!r) goto err;
  render_configure(r, render_width, render_height);
  for (;;) {
    if (window_should_close(w)) break;
    kp_update(kp);
    window_update(w);

    if (kp_getkey_press(kp, KP_KEY_ESC)) window_close(w);
    render_update(r);
  }
  render_del(r);
  kp_del(kp);
  window_del(w);
  return 0;

 err:
  window_del(w);
  kp_del(kp);
  render_del(r);
  return -1;
}
