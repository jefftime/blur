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

  int err;
  struct window w;
  struct kp_ctx kp;
  struct render_instance r;
  struct render_device device;
  struct render_pipeline pipeline;

  XRAND_SEED = (uint64_t) time(NULL);
  chkerrg(err = window_init(&w, "Blur", width, height), err_window);
  chkerrg(err = kp_init(&kp), err_kp);
  chkerrg(err = render_instance_init(&r, &w), err_render);
  chkerrg(err = render_device_init(&device, &r, 0), err_device);
  chkerrg(err = render_pipeline_init(&pipeline, &device), err_pipeline);
  for (;;) {
    if (w.should_close) break;
    kp_update(&kp);
    window_update(&w);

    if (kp_getkey_press(kp, KP_KEY_ESC)) w.should_close = 1;
    render_pipeline_update(&pipeline);
  }
  render_pipeline_deinit(&pipeline);
  render_device_deinit(&device);
  render_instance_deinit(&r);
  kp_deinit(&kp);
  window_deinit(&w);
  return 0;

 err_pipeline:
  render_device_deinit(&device);
 err_device:
  render_instance_deinit(&r);
 err_render:
  kp_deinit(&kp);
 err_kp:
  window_deinit(&w);
 err_window:
  return -1;
}
