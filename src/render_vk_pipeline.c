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

#include "render.h"
#include "error.h"
#include <stdlib.h>

/* **************************************** */
/* Public */
/* **************************************** */

int render_init_pipeline(
  struct render_pipeline *rp,
  struct render *r,
  size_t device,
  uint16_t width,
  uint16_t height
) {
  VkVertexInputBindingDescription bindings[] = {
    { 0, sizeof(float) * 6, VK_VERTEX_INPUT_RATE_VERTEX }
  };
  VkVertexInputAttributeDescription attrs[] = {
    { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },
    { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3 }
  };

  if (!rp) return RENDER_ERROR_NULL;
  if (!r) return RENDER_ERROR_NULL;
  rp->ctx = r;
  /* chkerrf(load_device_functions(rp), render_deinit_pipeline(rp)); */
  /* chkerrf(get_surface_format(r), render_deinit_pipeline(rp)); */
  /* chkerrf(create_swapchain(r), render_deinit_pipeline(rp)); */
  /* chkerrf( */
  /*   create_pipeline( */
  /*     r, */
  /*     sizeof(bindings) / sizeof(bindings[0]), */
  /*     bindings, */
  /*     sizeof(attrs) / sizeof(attrs[0]), */
  /*     attrs, */
  /*     vshader, */
  /*     fshader */
  /*   ), { */
  /*     render_deinit_pipeline(rp); */
  /*   } */
  /* ); */
  /* chkerrf(create_framebuffers(r), render_deinit_pipeline(rp)); */
  /* chkerrf(create_command_pool(r), render_deinit_pipeline(rp)); */
  /* chkerrf(create_command_buffers(r), render_deinit_pipeline(rp)); */
  return RENDER_ERROR_NONE;
}

void render_deinit_pipeline(struct render_pipeline *rp) {
  /* free(rp->queue_props); */
}
