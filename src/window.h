#ifndef WINDOW_H
#define WINDOW_H

#include "sized_types.h"

#define WINDOW_ERROR_NONE 0
#define WINDOW_ERROR_NULL -1

#ifdef PLATFORM_LINUX

#define WINDOW_ERROR_CONNECTION -2
#define WINDOW_ERROR_INTERN_ATOM -3

#include <xcb/xcb.h>
#include <xcb/shm.h>
struct window_os_details {
  int _initialized;
  xcb_connection_t *cn;
  xcb_gcontext_t gc;
  xcb_window_t wn;
  xcb_screen_t *screen;
  xcb_setup_t *setup;
  xcb_atom_t win_delete;
};

#else

struct window_os_details {
  int dummy;
};

#endif  /* PLATFORM_LINUX */

struct window {
  char *title;
  uint16_t width;
  uint16_t height;
  unsigned char should_close;

  struct window_os_details os;
};

/* **************************************** */
/* window_<platform>.c */
int window_init(struct window *, char *, uint16_t, uint16_t);
void window_deinit(struct window *);
void window_update(struct window *);
void window_close(struct window *);
void window_dimensions(struct window *, uint16_t *, uint16_t *);
/* **************************************** */

#endif
