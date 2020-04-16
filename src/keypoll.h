#ifndef KEYPOLL_H
#define KEYPOLL_H

struct kp_ctx;

enum kp_key_state {
  KP_STATE_UP = 0,
  KP_STATE_PRESSED = 1,
  KP_STATE_HELD = 2
};

enum kp_key {
  KP_KEY_BLANK = 0,
  KP_KEY_RESERVED,
  KP_KEY_ESC,
  KP_KEY_1,
  KP_KEY_2,
  KP_KEY_3,
  KP_KEY_4,
  KP_KEY_5,
  KP_KEY_6,
  KP_KEY_7,
  KP_KEY_8,
  KP_KEY_9,
  KP_KEY_0,
  KP_KEY_MINUS,
  KP_KEY_EQUAL,
  KP_KEY_BACKSPACE,
  KP_KEY_TAB,
  KP_KEY_Q,
  KP_KEY_W,
  KP_KEY_E,
  KP_KEY_R,
  KP_KEY_T,
  KP_KEY_Y,
  KP_KEY_U,
  KP_KEY_I,
  KP_KEY_O,
  KP_KEY_P,
  KP_KEY_LEFTBRACE,
  KP_KEY_RIGHTBRACE,
  KP_KEY_ENTER,
  KP_KEY_LEFTCTRL,
  KP_KEY_A,
  KP_KEY_S,
  KP_KEY_D,
  KP_KEY_F,
  KP_KEY_G,
  KP_KEY_H,
  KP_KEY_J,
  KP_KEY_K,
  KP_KEY_L,
  KP_KEY_SEMICOLON,
  KP_KEY_APOSTROPHE,
  KP_KEY_GRAVE,
  KP_KEY_LEFTSHIFT,
  KP_KEY_BACKSLASH,
  KP_KEY_Z,
  KP_KEY_X,
  KP_KEY_C,
  KP_KEY_V,
  KP_KEY_B,
  KP_KEY_N,
  KP_KEY_M,
  KP_KEY_COMMA,
  KP_KEY_DOT,
  KP_KEY_SLASH,
  KP_KEY_RIGHTSHIFT,
  KP_KEY_KPASTERISK,
  KP_KEY_LEFTALT,
  KP_KEY_SPACE,
  KP_KEY_CAPSLOCK,
  KP_KEY_F1,
  KP_KEY_F2,
  KP_KEY_F3,
  KP_KEY_F4,
  KP_KEY_F5,
  KP_KEY_F6,
  KP_KEY_F7,
  KP_KEY_F8,
  KP_KEY_F9,
  KP_KEY_F10,
  KP_KEY_NUMLOCK,
  KP_KEY_SCROLLLOCK,
  KP_KEY_KP7,
  KP_KEY_KP8,
  KP_KEY_KP9,
  KP_KEY_KPMINUS,
  KP_KEY_KP4,
  KP_KEY_KP5,
  KP_KEY_KP6,
  KP_KEY_KPPLUS,
  KP_KEY_KP1,
  KP_KEY_KP2,
  KP_KEY_KP3,
  KP_KEY_KP0,
  KP_KEY_KPDOT,
  KP_KEY_F11,
  KP_KEY_F12,
  KP_KEY_KPENTER,
  KP_KEY_RIGHTCTRL,
  KP_KEY_KPSLASH,
  KP_KEY_RIGHTALT,
  KP_KEY_LINEFEED,
  KP_KEY_HOME,
  KP_KEY_UP,
  KP_KEY_PAGEUP,
  KP_KEY_LEFT,
  KP_KEY_RIGHT,
  KP_KEY_END,
  KP_KEY_DOWN,
  KP_KEY_PAGEDOWN,
  KP_KEY_INSERT,
  KP_KEY_DELETE,
  KP_KEY_KPEQUAL,
  KP_KEY_PAUSE,
  KP_BTN_LEFT,
  KP_BTN_RIGHT,
  KP_BTN_MIDDLE,
  KP_BTN_SIDE,
  KP_BTN_EXTRA,
  KP_BTN_FORWARD,
  KP_BTN_BACK,
  KP_BTN_TASK,
  KP_BTN_TRIGGER,
  KP_BTN_THUMB,
  KP_BTN_THUMB2,
  KP_BTN_TOP,
  KP_BTN_TOP2,
  KP_BTN_PINKIE,
  KP_BTN_BASE,
  KP_BTN_BASE2,
  KP_BTN_BASE3,
  KP_BTN_BASE4,
  KP_BTN_BASE5,
  KP_BTN_BASE6,
  KP_BTN_DEAD,
  KP_BTN_SOUTH,
  KP_BTN_EAST,
  KP_BTN_NORTH,
  KP_BTN_WEST,
  KP_BTN_Z,
  KP_BTN_TL,
  KP_BTN_TR,
  KP_BTN_TL2,
  KP_BTN_TR2,
  KP_BTN_SELECT,
  KP_BTN_START,
  KP_BTN_MODE,
  KP_BTN_THUMBL,
  KP_BTN_THUMBR,
  KP_BTN_DPAD_UP,
  KP_BTN_DPAD_DOWN,
  KP_BTN_DPAD_LEFT,
  KP_BTN_DPAD_RIGHT,

  KP_MAX_KEYS
};

struct kp_ctx *kp_new(void);
void kp_del(struct kp_ctx *);
void kp_update(struct kp_ctx *);
int kp_getkey(struct kp_ctx *kp, enum kp_key key);
int kp_getkey_press(struct kp_ctx *kp, enum kp_key key);

#endif
