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

static int setup_connection(
  xcb_connection_t **out_cn,
  xcb_screen_t **out_screen
) {
  int i, screen_index;
  xcb_setup_t *setup;
  xcb_screen_iterator_t screen_iter;

  *out_cn = xcb_connect(NULL, &screen_index);
  if (!*out_cn) return WINDOW_ERROR_CONNECTION;
  setup = (xcb_setup_t *) xcb_get_setup(*out_cn);
  screen_iter = xcb_setup_roots_iterator(setup);
  /* get selected screen index information */
  for (i = 0; i < screen_index; ++i) {
    xcb_screen_next(&screen_iter);
  }
  *out_screen = screen_iter.data;
  return WINDOW_ERROR_NONE;
}

static int setup_window(
  xcb_connection_t *cn,
  char *title,
  uint16_t width,
  uint16_t height,
  xcb_screen_t *screen,
  xcb_window_t *out_wn
) {
  uint32_t mask = XCB_CW_EVENT_MASK;
  uint32_t values[] = { XCB_EVENT_MASK_STRUCTURE_NOTIFY };

  *out_wn = xcb_generate_id(cn);
  xcb_create_window(
    cn,
    XCB_COPY_FROM_PARENT,
    *out_wn,
    screen->root,
    0, 0,
    width, height,
    0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    screen->root_visual,
    mask,
    values
  );
  /* window title */
  xcb_change_property(
    cn,
    XCB_PROP_MODE_REPLACE,
    *out_wn,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING,
    8,
    (uint32_t) strlen(title),
    title
  );

  return WINDOW_ERROR_NONE;
}

static int setup_atoms(
  xcb_connection_t *cn,
  xcb_window_t wn,
  xcb_atom_t *out_win_delete
) {
  char *wm_protocols = "WM_PROTOCOLS";
  char *wm_delete_window = "WM_DELETE_WINDOW";
  xcb_intern_atom_cookie_t protocol_ck, delete_ck;
  xcb_intern_atom_reply_t *protocol, *delete;

  /* watch for delete event */
  protocol_ck = xcb_intern_atom(
    cn,
    0,
    (uint16_t) strlen(wm_protocols),
    wm_protocols
  );
  delete_ck = xcb_intern_atom(
    cn,
    0,
    (uint16_t) strlen(wm_delete_window),
    wm_delete_window
  );
  protocol = xcb_intern_atom_reply(cn, protocol_ck, NULL);
  delete = xcb_intern_atom_reply(cn, delete_ck, NULL);
  if (!protocol) return WINDOW_ERROR_INTERN_ATOM;
  if (!delete) {
    free(protocol);
    return WINDOW_ERROR_INTERN_ATOM;
  }
  xcb_change_property(
    cn,
    XCB_PROP_MODE_REPLACE,
    wn,
    protocol->atom,
    XCB_ATOM_ATOM,
    32,
    1,
    &delete->atom
  );
  *out_win_delete = delete->atom;
  free(protocol);
  free(delete);
  xcb_map_window(cn, wn);
  return WINDOW_ERROR_NONE;
}

static int init_xcb(
  char *title,
  uint16_t width,
  uint16_t height,
  xcb_connection_t **out_cn,
  xcb_window_t *out_wn,
  xcb_atom_t *out_win_delete
) {
  xcb_screen_t *screen;

  chkerr(setup_connection(out_cn, &screen));
  chkerr(setup_window(*out_cn, title, width, height, screen, out_wn));
  chkerr(setup_atoms(*out_cn, *out_wn, out_win_delete));
  xcb_flush(*out_cn);
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
  chkerr(
    init_xcb(
      title,
      width,
      height,
      &w->os.cn,
      &w->os.wn,
      &w->os.win_delete
    )
  );
  return WINDOW_ERROR_NONE;
}

void window_deinit(struct window *w) {
  if (!w) return;
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
