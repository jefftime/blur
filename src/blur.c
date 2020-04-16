#include "error.h"
#include "window.h"             /* window_new, window_del */
#include "keypoll.h"            /* kp_new, kp_del */
#include <stdio.h>              /* printf, fflush */

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
    int32_t x, y, z, rx, ry, rz;

    if (window_should_close(w)) break;
    kp_update(kp);
    window_update(w);

    if (kp_getkey(kp, KP_KEY_Q)) puts("Q");
    if (kp_getkey_press(kp, KP_KEY_A)) puts("A");
    if (kp_getkey_press(kp, KP_BTN_LEFT)) puts("BTN_LEFT");
    if (kp_getkey_press(kp, KP_BTN_SOUTH)) puts("BTN_SOUTH");
    if (kp_getkey_press(kp, KP_BTN_NORTH)) puts("BTN_NORTH");
    if (kp_getkey_press(kp, KP_BTN_WEST)) puts("BTN_WEST");
    if (kp_getkey_press(kp, KP_BTN_EAST)) puts("BTN_EAST");
    kp_getpos_analogs(kp, &x, &y, &z, &rx, &ry, &rz);
    printf("%d %d %d | %d %d %d\n", x, y, z, rx, ry, rz);
    fflush(stdout);
  }
  window_del(w);
  kp_del(kp);
  return 0;

 err:
  window_del(w);
  kp_del(kp);
  return -1;
}
