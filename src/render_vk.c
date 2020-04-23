#include "render.h"
#include "window.h"
#include <stdlib.h>

struct render {
  int x;
};

struct render *render_new(struct window *w) {
  struct render *out;

  out = malloc(sizeof(struct render));
  if (!out) return NULL;
  return out;

 err:
  return NULL;
}

void render_del(struct render *r) {
  if (!r) return;
  free(r);
}

int render_configure(struct render *r, uint16_t width, uint16_t height) {
  return 0;
}

void render_update(struct render *r) {
  
}
