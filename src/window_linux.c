#include "window.h"
#include "sized_types.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>

struct window {
  char *title;
  uint16_t width;
  uint16_t height;
  unsigned char should_close;

  /* xcb */
  xcb_connection_t *cn;
  xcb_gcontext_t gc;
  xcb_window_t wn;
  xcb_screen_t *screen;
  xcb_setup_t *setup;
  xcb_atom_t win_delete;
};

static int setup_connection(struct window *w) {
  int i, screen_index;
  xcb_screen_iterator_t screen_iter;

  w->cn = xcb_connect(NULL, &screen_index);
  if (!w->cn) return WINDOW_ERROR_CONNECTION;
  w->setup = (xcb_setup_t *) xcb_get_setup(w->cn);
  screen_iter = xcb_setup_roots_iterator(w->setup);
  /* get selected screen index information */
  for (i = 0; i < screen_index; ++i) {
    xcb_screen_next(&screen_iter);
  }
  w->screen = screen_iter.data;
  return 0;
}

static int setup_window(struct window *w) {
  char *wm_protocols = "WM_PROTOCOLS";
  char *wm_delete_window = "WM_DELETE_WINDOW";
  uint32_t mask = XCB_CW_EVENT_MASK;
  uint32_t values[] = { XCB_EVENT_MASK_STRUCTURE_NOTIFY };
  xcb_intern_atom_cookie_t protocol_ck, delete_ck;
  xcb_intern_atom_reply_t *protocol, *delete;

  w->wn = xcb_generate_id(w->cn);
  xcb_create_window(
    w->cn,
    XCB_COPY_FROM_PARENT,
    w->wn,
    w->screen->root,
    0, 0,
    w->width, w->height,
    0,
    XCB_WINDOW_CLASS_INPUT_OUTPUT,
    w->screen->root_visual,
    mask,
    values
  );
  /* window title */
  xcb_change_property(
    w->cn,
    XCB_PROP_MODE_REPLACE,
    w->wn,
    XCB_ATOM_WM_NAME,
    XCB_ATOM_STRING,
    8,
    (uint32_t) strlen(w->title),
    w->title
  );
  /* watch for delete event */
  protocol_ck = xcb_intern_atom(
    w->cn,
    0,
    (uint16_t) strlen(wm_protocols),
    wm_protocols
  );
  delete_ck = xcb_intern_atom(
    w->cn,
    0,
    (uint16_t) strlen(wm_delete_window),
    wm_delete_window
  );
  protocol = xcb_intern_atom_reply(w->cn, protocol_ck, NULL);
  delete = xcb_intern_atom_reply(w->cn, delete_ck, NULL);
  if (!protocol) return WINDOW_ERROR_INTERN_ATOM;
  if (!delete) {
    free(protocol);
    return WINDOW_ERROR_INTERN_ATOM;
  }
  xcb_change_property(
    w->cn,
    XCB_PROP_MODE_REPLACE,
    w->wn,
    protocol->atom,
    XCB_ATOM_ATOM,
    32,
    1,
    &delete->atom
  );
  w->win_delete = delete->atom;
  free(protocol);
  free(delete);
  xcb_map_window(w->cn, w->wn);
  return 0;
}

static int setup_gc(struct window *w) {
  uint32_t values[] = { 0 };

  w->gc = xcb_generate_id(w->cn);
  xcb_create_gc(
    w->cn,
    w->gc,
    w->wn,
    XCB_GC_GRAPHICS_EXPOSURES,
    values
  );
  return 0;
}

static int init_xcb(struct window *w) {
  chkerr(setup_connection(w));
  chkerr(setup_window(w));
  chkerr(setup_gc(w));
  xcb_flush(w->cn);
  return 0;
}

/* **************************************** */
/* Public */
/* **************************************** */

struct window *window_new(
  char *title,
  uint16_t width,
  uint16_t height,
  int *err
) {
  int rc = 0;
  struct window *out;

  out = malloc(sizeof(struct window));
  if (!out) return NULL;
  memset(out, 1, sizeof(struct window));
  out->title = title;
  out->width = width;
  out->height = height;
  if ((rc = init_xcb(out))) goto err;
  out->should_close = 0;
  return out;

 err:
  if (out) free(out);
  if (err) *err = rc;
  return NULL;
}

void window_del(struct window *w) {
  if (!w) return;

  xcb_destroy_window(w->cn, w->wn);
  xcb_flush(w->cn);
  xcb_disconnect(w->cn);
  free(w);
}

void window_update(struct window *w) {
  xcb_generic_event_t *event;

  if (!w) return;
  while ((event = xcb_poll_for_event(w->cn))) {
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
      if (e->data.data32[0] == w->win_delete) w->should_close = 1;
      break;
    }
    }
    free(event);
  }
}

void window_close(struct window *w) {
  if (!w) return;
  w->should_close = 1;
}

int window_should_close(struct window *w) {
  if (!w) return 0;
  return w->should_close;
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

int window_get_os_details(struct window *w, struct window_os_details *out) {
  if (!w) return WINDOW_ERROR_NULL;
  if (!out) return WINDOW_ERROR_NULL;

  out->cn = w->cn;
  out->wn = w->wn;
  return 0;
}
