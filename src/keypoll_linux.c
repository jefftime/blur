#include "keypoll.h"
#include "sized_types.h"
#include <linux/input.h>        /* struct input_event */
#include <dirent.h>             /* opendir, readdir */
#include <fcntl.h>              /* open */
#include <stddef.h>             /* size_t */
#include <stdlib.h>             /* free */
#include <string.h>             /* strlen, strcat */
#include <unistd.h>             /* close */

enum {
  MAX_DEVICES = 32
};

struct kp_ctx {
  size_t n_devices;
  int fds[MAX_DEVICES];
  unsigned char keymap[KP_MAX_KEYS];
  struct {
    int32_t x;
    int32_t y;
  } mouse;
  struct {
    int32_t stick_x;
    int32_t stick_y;
    int32_t trigger;
  } left;
  struct {
    int32_t stick_x;
    int32_t stick_y;
    int32_t trigger;
  } right;
};

size_t get_key_index(int code) {
  switch (code) {
  case KEY_RESERVED: return KP_KEY_RESERVED;
  case KEY_ESC: return KP_KEY_ESC;
  case KEY_1: return KP_KEY_1;
  case KEY_2: return KP_KEY_2;
  case KEY_3: return KP_KEY_3;
  case KEY_4: return KP_KEY_4;
  case KEY_5: return KP_KEY_5;
  case KEY_6: return KP_KEY_6;
  case KEY_7: return KP_KEY_7;
  case KEY_8: return KP_KEY_8;
  case KEY_9: return KP_KEY_9;
  case KEY_0: return KP_KEY_0;
  case KEY_MINUS: return KP_KEY_MINUS;
  case KEY_EQUAL: return KP_KEY_EQUAL;
  case KEY_BACKSPACE: return KP_KEY_BACKSPACE;
  case KEY_TAB: return KP_KEY_TAB;
  case KEY_Q: return KP_KEY_Q;
  case KEY_W: return KP_KEY_W;
  case KEY_E: return KP_KEY_E;
  case KEY_R: return KP_KEY_R;
  case KEY_T: return KP_KEY_T;
  case KEY_Y: return KP_KEY_Y;
  case KEY_U: return KP_KEY_U;
  case KEY_I: return KP_KEY_I;
  case KEY_O: return KP_KEY_O;
  case KEY_P: return KP_KEY_P;
  case KEY_LEFTBRACE: return KP_KEY_LEFTBRACE;
  case KEY_RIGHTBRACE: return KP_KEY_RIGHTBRACE;
  case KEY_ENTER: return KP_KEY_ENTER;
  case KEY_LEFTCTRL: return KP_KEY_LEFTCTRL;
  case KEY_A: return KP_KEY_A;
  case KEY_S: return KP_KEY_S;
  case KEY_D: return KP_KEY_D;
  case KEY_F: return KP_KEY_F;
  case KEY_G: return KP_KEY_G;
  case KEY_H: return KP_KEY_H;
  case KEY_J: return KP_KEY_J;
  case KEY_K: return KP_KEY_K;
  case KEY_L: return KP_KEY_L;
  case KEY_SEMICOLON: return KP_KEY_SEMICOLON;
  case KEY_APOSTROPHE: return KP_KEY_APOSTROPHE;
  case KEY_GRAVE: return KP_KEY_GRAVE;
  case KEY_LEFTSHIFT: return KP_KEY_LEFTSHIFT;
  case KEY_BACKSLASH: return KP_KEY_BACKSLASH;
  case KEY_Z: return KP_KEY_Z;
  case KEY_X: return KP_KEY_X;
  case KEY_C: return KP_KEY_C;
  case KEY_V: return KP_KEY_V;
  case KEY_B: return KP_KEY_B;
  case KEY_N: return KP_KEY_N;
  case KEY_M: return KP_KEY_M;
  case KEY_COMMA: return KP_KEY_COMMA;
  case KEY_DOT: return KP_KEY_DOT;
  case KEY_SLASH: return KP_KEY_SLASH;
  case KEY_RIGHTSHIFT: return KP_KEY_RIGHTSHIFT;
  case KEY_KPASTERISK: return KP_KEY_KPASTERISK;
  case KEY_LEFTALT: return KP_KEY_LEFTALT;
  case KEY_SPACE: return KP_KEY_SPACE;
  case KEY_CAPSLOCK: return KP_KEY_CAPSLOCK;
  case KEY_F1: return KP_KEY_F1;
  case KEY_F2: return KP_KEY_F2;
  case KEY_F3: return KP_KEY_F3;
  case KEY_F4: return KP_KEY_F4;
  case KEY_F5: return KP_KEY_F5;
  case KEY_F6: return KP_KEY_F6;
  case KEY_F7: return KP_KEY_F7;
  case KEY_F8: return KP_KEY_F8;
  case KEY_F9: return KP_KEY_F9;
  case KEY_F10: return KP_KEY_F10;
  case KEY_NUMLOCK: return KP_KEY_NUMLOCK;
  case KEY_SCROLLLOCK: return KP_KEY_SCROLLLOCK;
  case KEY_KP7: return KP_KEY_KP7;
  case KEY_KP8: return KP_KEY_KP8;
  case KEY_KP9: return KP_KEY_KP9;
  case KEY_KPMINUS: return KP_KEY_KPMINUS;
  case KEY_KP4: return KP_KEY_KP4;
  case KEY_KP5: return KP_KEY_KP5;
  case KEY_KP6: return KP_KEY_KP6;
  case KEY_KPPLUS: return KP_KEY_KPPLUS;
  case KEY_KP1: return KP_KEY_KP1;
  case KEY_KP2: return KP_KEY_KP2;
  case KEY_KP3: return KP_KEY_KP3;
  case KEY_KP0: return KP_KEY_KP0;
  case KEY_KPDOT: return KP_KEY_KPDOT;
  case KEY_F11: return KP_KEY_F11;
  case KEY_F12: return KP_KEY_F12;
  case KEY_KPENTER: return KP_KEY_KPENTER;
  case KEY_RIGHTCTRL: return KP_KEY_RIGHTCTRL;
  case KEY_KPSLASH: return KP_KEY_KPSLASH;
  case KEY_RIGHTALT: return KP_KEY_RIGHTALT;
  case KEY_LINEFEED: return KP_KEY_LINEFEED;
  case KEY_HOME: return KP_KEY_HOME;
  case KEY_UP: return KP_KEY_UP;
  case KEY_PAGEUP: return KP_KEY_PAGEUP;
  case KEY_LEFT: return KP_KEY_LEFT;
  case KEY_RIGHT: return KP_KEY_RIGHT;
  case KEY_END: return KP_KEY_END;
  case KEY_DOWN: return KP_KEY_DOWN;
  case KEY_PAGEDOWN: return KP_KEY_PAGEDOWN;
  case KEY_INSERT: return KP_KEY_INSERT;
  case KEY_DELETE: return KP_KEY_DELETE;
  case KEY_KPEQUAL: return KP_KEY_KPEQUAL;
  case KEY_PAUSE: return KP_KEY_PAUSE;
  case BTN_LEFT: return KP_BTN_LEFT;
  case BTN_RIGHT: return KP_BTN_RIGHT;
  case BTN_MIDDLE: return KP_BTN_MIDDLE;
  case BTN_SIDE: return KP_BTN_SIDE;
  case BTN_EXTRA: return KP_BTN_EXTRA;
  case BTN_FORWARD: return KP_BTN_FORWARD;
  case BTN_BACK: return KP_BTN_BACK;
  case BTN_TASK: return KP_BTN_TASK;
  case BTN_TRIGGER: return KP_BTN_TRIGGER;
  case BTN_THUMB: return KP_BTN_THUMB;
  case BTN_THUMB2: return KP_BTN_THUMB2;
  case BTN_TOP: return KP_BTN_TOP;
  case BTN_TOP2: return KP_BTN_TOP2;
  case BTN_PINKIE: return KP_BTN_PINKIE;
  case BTN_BASE: return KP_BTN_BASE;
  case BTN_BASE2: return KP_BTN_BASE2;
  case BTN_BASE3: return KP_BTN_BASE3;
  case BTN_BASE4: return KP_BTN_BASE4;
  case BTN_BASE5: return KP_BTN_BASE5;
  case BTN_BASE6: return KP_BTN_BASE6;
  case BTN_DEAD: return KP_BTN_DEAD;
  case BTN_SOUTH: return KP_BTN_SOUTH;
  case BTN_EAST: return KP_BTN_EAST;
  case BTN_NORTH: return KP_BTN_NORTH;
  case BTN_WEST: return KP_BTN_WEST;
  case BTN_Z: return KP_BTN_Z;
  case BTN_TL: return KP_BTN_TL;
  case BTN_TR: return KP_BTN_TR;
  case BTN_TL2: return KP_BTN_TL2;
  case BTN_TR2: return KP_BTN_TR2;
  case BTN_SELECT: return KP_BTN_SELECT;
  case BTN_START: return KP_BTN_START;
  case BTN_MODE: return KP_BTN_MODE;
  case BTN_THUMBL: return KP_BTN_THUMBL;
  case BTN_THUMBR: return KP_BTN_THUMBR;
  case BTN_DPAD_UP: return KP_BTN_DPAD_UP;
  case BTN_DPAD_DOWN: return KP_BTN_DPAD_DOWN;
  case BTN_DPAD_LEFT: return KP_BTN_DPAD_LEFT;
  case BTN_DPAD_RIGHT: return KP_BTN_DPAD_RIGHT;
  }
  return KP_KEY_BLANK;
}

int supported_device(char *name) {
  char *supported_devices[] = {
    "event-kbd",
    "event-mouse",
    "event-joystick"
  };
  size_t i, n_devices, len;

  len = strlen(name);
  n_devices = sizeof(supported_devices) / sizeof(supported_devices[0]);
  for (i = 0; i < n_devices; ++i) {
    char *device;
    int match;

    device = supported_devices[i];
    match = !strcmp(name + (len - strlen(device)), device);
    if (match) return 1;
  }
  return 0;
}

int process_devices(struct kp_ctx *kp, size_t n_devices, char **devices) {
  enum {
    filepath_end = 19
  };

  char filepath[256] = "/dev/input/by-path/";
  size_t i;

  kp->n_devices = n_devices;
  for (i = 0; i < n_devices; ++i) {
    strcat(filepath, devices[i]);
    kp->fds[i] = open(filepath, O_RDONLY | O_NONBLOCK);
    /* we're going to reuse filepath for each device */
    filepath[filepath_end] = '\0';
    if (kp->fds[i] < 0) goto err;
    continue;

  err:
    /* close all previous file descriptors */
    while (i--) close(kp->fds[i]);
    return -1;
  }
  return 0;
}

void update_states(struct kp_ctx *kp) {
  size_t i;

  /* advance the state from KP_STATE_PRESSED to KP_STATE_HELD */
  for (i = 0; i < KP_MAX_KEYS; ++i) {
    kp->keymap[i] += (kp->keymap[i] == 1);
  }
}

void set_keymap(struct kp_ctx *kp, int code, int value) {
  /*
   * On Linux, key repeat events are 2 which is the same as
   * our KP_STATE_PRESSED, so we don't have to filter them out
   * here
   */
  kp->keymap[get_key_index(code)] = (unsigned char) value;
}

/* **************************************** */
/* Public */
/* **************************************** */

struct kp_ctx *kp_new(void) {
  char *devices[MAX_DEVICES] = { 0 };
  size_t n_devices = 0;
  DIR *input_dir = NULL;
  struct dirent *input = NULL;
  struct kp_ctx *kp;

  kp = malloc(sizeof(struct kp_ctx));
  if (!kp) return NULL;
  input_dir = opendir("/dev/input/by-path");
  if (!input_dir) goto err;
  while ((input = readdir(input_dir))) {
    if (!supported_device(input->d_name)) continue;
    if (n_devices >= MAX_DEVICES) break;
    devices[n_devices++] = input->d_name;
  }
  if (process_devices(kp, n_devices, devices)) goto err;
  closedir(input_dir);
  return kp;

 err:
  free(kp);
  return NULL;
}

void kp_del(struct kp_ctx *kp) {
  size_t i;

  if (!kp) return;
  for (i = 0; i < kp->n_devices; ++i) close(kp->fds[i]);
  free(kp);
}

void kp_update(struct kp_ctx *kp) {
  /* no null check */
  size_t i;

  update_states(kp);
  for (i = 0; i < kp->n_devices; ++i) {
    ssize_t rc;
    struct input_event e;

    while ((rc = read(kp->fds[i], &e, sizeof(struct input_event))) > 0) {
      switch (e.type) {
      case EV_KEY:
        set_keymap(kp, e.code, e.value);
        break;

        /* gamepad analog sticks/triggers */
      case EV_ABS:
        switch (e.code) {
        case ABS_MT_POSITION_X: kp->mouse.x = e.value; break;
        case ABS_MT_POSITION_Y: kp->mouse.y = e.value; break;
        case ABS_X: kp->left.stick_x = e.value; break;
        case ABS_Y: kp->left.stick_y = e.value; break;
        case ABS_Z: kp->left.trigger = e.value; break;
        case ABS_RX: kp->right.stick_x = e.value; break;
        case ABS_RY: kp->right.stick_y = e.value; break;
        case ABS_RZ: kp->right.trigger = e.value; break;
        }
        break;

        /* mouse movement */
      case EV_REL:
        break;
      }
    }
  }
}

int kp_getkey(struct kp_ctx *kp, enum kp_key key) {
  /* no null check */
  return kp->keymap[key];
}

int kp_getkey_press(struct kp_ctx *kp, enum kp_key key) {
  return kp_getkey(kp, key) == KP_STATE_PRESSED;
}

void kp_getpos_mouse(struct kp_ctx *kp, int32_t *out_x, int32_t *out_y) {
  if (out_x) *out_x = kp->mouse.x;
  if (out_y) *out_y = kp->mouse.y;
}

void kp_getpos_analogs(
  struct kp_ctx *kp,
  int32_t *out_x,
  int32_t *out_y,
  int32_t *out_z,
  int32_t *out_rx,
  int32_t *out_ry,
  int32_t *out_rz
) {
  if (out_x) *out_x = kp->left.stick_x;
  if (out_y) *out_y = kp->left.stick_y;
  if (out_z) *out_z = kp->left.trigger;
  if (out_rx) *out_rx = kp->right.stick_x;
  if (out_ry) *out_ry = kp->right.stick_y;
  if (out_rz) *out_rz = kp->right.trigger;
}
