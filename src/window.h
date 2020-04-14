#ifndef WINDOW_H
#define WINDOW_H

#include "sized_types.h"

#ifdef PLATFORM_LINUX
#include <xcb/xcb.h>
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
int window_should_close(struct window *);
int window_get_os_details(struct window *, struct window_os_details *);
/* **************************************** */

/* **************************************** */
/* window_key.c */

/* **************************************** */

#endif
