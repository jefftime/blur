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

#define WINDOW_ERROR_NULL -1
#define WINDOW_ERROR_CONNECTION -2
#define WINDOW_ERROR_INTERN_ATOM -3

/* **************************************** */
/* window_<platform>.c */
struct window *window_new(char *, uint16_t, uint16_t, int *);
void window_del(struct window *);
void window_update(struct window *);
void window_close(struct window *);
int window_should_close(struct window *);
void window_dimensions(struct window *, uint16_t *, uint16_t *);
int window_get_os_details(struct window *, struct window_os_details *);
/* **************************************** */

#endif
