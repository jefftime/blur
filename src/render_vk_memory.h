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

#ifndef RENDER_VK_MEMORY_H
#define RENDER_VK_MEMORY_H

#include "render.h"

int create_buffer(
  struct render_pipeline *rp,
  VkBuffer *out_buf,
  size_t size,
  VkBufferUsageFlags flags
);
int alloc_buffer(
  struct render_pipeline *rp,
  VkBuffer buf,
  VkDeviceMemory *out_mem
);
int write_data(
  struct render_pipeline *rp,
  VkDeviceMemory mem,
  size_t size,
  void *data
);

#endif
