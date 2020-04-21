#ifndef WINDOW_H
#define WINDOW_H

#include "sized_types.h"

#ifdef PLATFORM_LINUX
# include <xcb/xcb.h>
# include <xcb/shm.h>
struct window_os_details {
  xcb_connection_t *cn;
  xcb_window_t wn;
};
#endif

struct window;

/* **************************************** */
/* window_<platform>.c */
struct window *window_new(char *, uint16_t, uint16_t);
void window_del(struct window *);
void window_update(struct window *);
void window_close(struct window *);
int window_should_close(struct window *);
void window_dimensions(struct window *, uint16_t *, uint16_t *);
uint32_t *window_buffer(struct window *);
void window_draw(struct window *);
int window_get_os_details(struct window *, struct window_os_details *);
/* **************************************** */

#endif
