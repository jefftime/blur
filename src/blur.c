/* Copyright 2020, Jeffery Stager
 *
 * This file is part of Blur
 *
 * Blur is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Blur is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Blur.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "error.h"
#include "sized_types.h"
#include "window.h"
#include "keypoll.h"
#include "render.h"
#include "xrand.h"
#include <time.h>

int main(int argc, char **argv) {
  enum {
    width = 960,
    height = 720,
    render_width = 320,
    render_height = 240
  };

  struct window w;
  struct kp_ctx kp;
  struct render r;

  XRAND_SEED = (uint64_t) time(NULL);
  if (window_init(&w, "Blur", width, height)) goto err;
  if (kp_init(&kp)) goto err;
  if (render_init(&r, &w)) goto err;
  render_configure(&r, render_width, render_height);
  for (;;) {
    if (w.should_close) break;
    kp_update(&kp);
    window_update(&w);

    if (kp_getkey_press(kp, KP_KEY_ESC)) w.should_close = 1;
    render_update(&r);
  }
  render_deinit(&r);
  kp_deinit(&kp);
  window_deinit(&w);
  return 0;

 err:
  window_deinit(&w);
  kp_deinit(&kp);
  render_deinit(&r);
  return -1;
}
