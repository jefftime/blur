#include "render.h"
#include "window.h"
#include "xrand.h"
#include "sized_types.h"
#include <stdlib.h>
#include <string.h>

struct render {
  struct window *window;
  uint16_t width;
  uint16_t height;
  uint16_t window_width;
  uint16_t window_height;
  uint32_t *palette;
  unsigned char *scr;
};

uint32_t *init_palette(void) {
  enum {
    max_colors = 256
  };

  uint32_t *palette;
  size_t i;

  palette = calloc(1, sizeof(int32_t) * max_colors);
  if (!palette) return NULL;
  for (i = 0; i < max_colors; ++i) {
    unsigned char r, g, b;

    r = (unsigned char) ((i % 16) * 256 / 16);
    g = (unsigned char) ((i / 16) * 256 / 16);
    b = (unsigned char) ((i % 8) * 256 / 8);
    palette[i] |= (uint32_t) r << 16;
    palette[i] |= (uint32_t) g << 8;
    palette[i] |= (uint32_t) b;
  }
  return palette;
}

static void scale_centered(struct render *r) {
  uint32_t *buf;
  size_t i;

  buf = window_buffer(r->window);
  for (i = 0; i < r->width * r->height; ++i) {
    buf[i] = r->palette[r->scr[i]];
  }
}

/* **************************************** */
/* Public */
/* **************************************** */

struct render *render_new(struct window *w) {
  uint16_t window_width, window_height;
  struct render *out;

  out = malloc(sizeof(struct render));
  if (!out) return NULL;
  memset(out, 0, sizeof(struct render));
  window_dimensions(w, &window_width, window_height);
  out->window = w;
  out->palette = init_palette();
  out->window_width = window_width;
  out->window_height = window_height;
  if (!out->palette) goto err;
  return out;

 err:
  free(out);
  return NULL;
}

void render_del(struct render *r) {
  if (!r) return;
  free(r->scr);
  free(r);
}

int render_configure(struct render *r, uint16_t width, uint16_t height) {
  if (!r) return -1;
  if (r->scr) free(r->scr);
  r->width = width;
  r->height = height;
  r->scr = malloc(width * height);
  if (!r->scr) return -1;
  return 0;
}

void render_update(struct render *r) {
  size_t i, j;

  for (i = 0; i < r->width * r->height; ++i) {
    r->scr[i] = (unsigned char) (xrand() % 256);
  }
  scale_centered(r);
  window_draw(r->window);
}

void render_line(
  struct render *r,
  int32_t x1,
  int32_t y1,
  int32_t x2,
  int32_t y2
) {

}

