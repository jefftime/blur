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
    palette[i] = (i << 16) | (i << 8) | i;
  }
  return palette;
}

static void scale_centered(struct render *r) {
  unsigned char width_scale, height_scale, scale;
  uint16_t window_width, window_height;
  uint32_t *buf;
  int32_t horiz_offset, vert_offset;
  size_t i, j, s1, s2;

  window_dimensions(r->window, &window_width, &window_height);
  width_scale = window_width / r->width;
  height_scale = window_height / r->height;
  scale = width_scale < height_scale ? width_scale : height_scale;
  buf = window_buffer(r->window);
  horiz_offset = (window_width - (r->width * scale)) / 2;
  vert_offset = (window_width) * ((window_height - (r->height * scale)) / 2);
  buf += horiz_offset + vert_offset;
  for (i = 0; i < r->height; ++i) {
    for (s1 = 0; s1 < scale; ++s1) {
      for (j = 0; j < r->width; ++j) {
        for (s2 = 0; s2 < scale; ++s2) {
          *buf++ = r->palette[r->scr[(i * r->width) + j]];
        }
      }
      buf += window_width - (r->width * scale);
    }
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
  out->window = w;
  out->palette = init_palette();
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
  size_t i;

  if (!r) return -1;
  if (r->scr) free(r->scr);
  r->width = width;
  r->height = height;
  r->scr = malloc(width * height);
  if (!r->scr) return -1;
  for (i = 0; i < r->width * r->height; ++i) {
    r->scr[i] = (unsigned char) (xrand() % 256);
  }
  return 0;
}

void render_update(struct render *r) {
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

