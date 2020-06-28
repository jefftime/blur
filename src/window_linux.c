/* Copyright 2020, Jeffery Stager
 *
 * This file is part of Tortuga
 *
 * Tortuga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tortuga is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tortuga.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "window.h"
#include "sized_types.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>

static int setup_connection(struct window *w) {
  int i, screen_index;
  xcb_screen_iterator_t screen_iter;

  w->os.cn = xcb_connect(NULL, &screen_index);
  if (!w->os.cn) return WINDOW_ERROR_CONNECTION;
  w->os.setup = (xcb_setup_t *) xcb_get_setup(w->os.cn);
  screen_iter = xcb_setup_roots_iterator(w->os.setup);
  /* get selected screen index information */
  for (i = 0; i < screen_index; ++i) {
    xcb_screen_next(&screen_iter);
  }
  w->os.screen = screen_iter.data;
  return 0;
}

static int setup_window(struct window *w) {
  char *wm_protocols = "WM_PROTOCOLS";
  char *wm_delete_window = "WM_DELETE_WINDOW";
  uint32_t mask = XCB_CW_EVENT_MASK;
  uint32_t values[] = { XCB_EVENT_MASK_STRUCTURE_NOTIFY };
  xcb_intern_atom_cookie_t protocol_ck, delete_ck;
  xcb_intern_atom_reply_t *protocol, *delete;

  w->os.wn = xcb_generate_id(w->os.cn);
  xcb_create_window(
    w->os.cn,
    XCB_COPY_FROM_PARENT,
    w->os.wn,
    w->os.screen->root,
    0, 0,
    w->width, w->height,
    0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    w->os.screen->root_visual,
    mask,
    values
  );
  /* window title */
  xcb_change_property(
    w->os.cn,
    XCB_PROP_MODE_REPLACE,
    w->os.wn,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING,
    8,
    (uint32_t) strlen(w->title),
    w->title
  );
  /* watch for delete event */
  protocol_ck = xcb_intern_atom(
    w->os.cn,
    0,
    (uint16_t) strlen(wm_protocols),
    wm_protocols
  );
  delete_ck = xcb_intern_atom(
    w->os.cn,
    0,
    (uint16_t) strlen(wm_delete_window),
    wm_delete_window
  );
  protocol = xcb_intern_atom_reply(w->os.cn, protocol_ck, NULL);
  delete = xcb_intern_atom_reply(w->os.cn, delete_ck, NULL);
  if (!protocol) return WINDOW_ERROR_INTERN_ATOM;
  if (!delete) {
    free(protocol);
    return WINDOW_ERROR_INTERN_ATOM;
  }
  xcb_change_property(
    w->os.cn,
    XCB_PROP_MODE_REPLACE,
    w->os.wn,
    protocol->atom,
    XCB_ATOM_ATOM,
    32,
    1,
    &delete->atom
  );
  w->os.win_delete = delete->atom;
  free(protocol);
  free(delete);
  xcb_map_window(w->os.cn, w->os.wn);
  return 0;
}

static int setup_gc(struct window *w) {
  uint32_t values[] = { 0 };

  w->os.gc = xcb_generate_id(w->os.cn);
  xcb_create_gc(
    w->os.cn,
    w->os.gc,
    w->os.wn,
    XCB_GC_GRAPHICS_EXPOSURES,
    values
  );
  return 0;
}

static int init_xcb(struct window *w) {
  chkerr(setup_connection(w));
  chkerr(setup_window(w));
  chkerr(setup_gc(w));
  xcb_flush(w->os.cn);
  return 0;
}

/* **************************************** */
/* Public */
/* **************************************** */

int window_init(
  struct window *w,
  char *title,
  uint16_t width,
  uint16_t height
) {
  if (!w) return WINDOW_ERROR_NULL;
  memset(w, 0, sizeof(struct window));
  w->title = title;
  w->width = width;
  w->height = height;
  w->should_close = 0;
  chkerr(init_xcb(w));
  w->os._initialized = 1;
  return WINDOW_ERROR_NONE;
}

void window_deinit(struct window *w) {
  if (!w) return;
  if (!w->os._initialized) return;
  xcb_destroy_window(w->os.cn, w->os.wn);
  xcb_flush(w->os.cn);
  xcb_disconnect(w->os.cn);
}

void window_update(struct window *w) {
  xcb_generic_event_t *event;

  if (!w) return;
  while ((event = xcb_poll_for_event(w->os.cn))) {
    switch (event->response_type & ~0x80) {
    case XCB_CONFIGURE_NOTIFY: {
      /* resize */
      xcb_configure_notify_event_t *e;

      e = (xcb_configure_notify_event_t *) event;
      if ((e->width != w->width) || (e->height != w->height)) {
        w->width = e->width;
        w->height = e->height;
      }
      break;
    }

    case XCB_CLIENT_MESSAGE: {
      /* close window */
      xcb_client_message_event_t *e;

      e = (xcb_client_message_event_t *) event;
      if (e->data.data32[0] == w->os.win_delete) w->should_close = 1;
      break;
    }
    }
    free(event);
  }
}

void window_dimensions(
  struct window *w,
  uint16_t *out_width,
  uint16_t *out_height
) {
  if (!w) return;
  if (out_width) *out_width = w->width;
  if (out_height) *out_height = w->height;
}
