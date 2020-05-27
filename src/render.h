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

#ifndef RENDER_H
#define RENDER_H

#include "window.h"

#ifdef RENDER_BACKEND_VULKAN
#include "render_vk.h"
#endif

#define RENDER_ERROR_NONE 0
#define RENDER_ERROR_NULL -1
#define RENDER_ERROR_MEMORY -2
#define RENDER_ERROR_VULKAN_LOAD -3
#define RENDER_ERROR_VULKAN_LOAD_INSTANCE_FUNCTION -4
#define RENDER_ERROR_VULKAN_CREATE_INSTANCE -5
#define RENDER_ERROR_VULKAN_INSTANCE_FUNCTIONS -6
#define RENDER_ERROR_VULKAN_SURFACE -7
#define RENDER_ERROR_VULKAN_PHYSICAL_DEVICES -8
#define RENDER_ERROR_VULKAN_INVALID_DEVICE -9
#define RENDER_ERROR_VULKAN_QUEUE_INDICES -10
#define RENDER_ERROR_VULKAN_DEVICE -11
#define RENDER_ERROR_VULKAN_DEVICE_FUNCTION -12
#define RENDER_ERROR_VULKAN_SURFACE_CAPABILITIES -13
#define RENDER_ERROR_VULKAN_SWAPCHAIN -14
#define RENDER_ERROR_VULKAN_SWAPCHAIN_IMAGES -15
#define RENDER_ERROR_VULKAN_SEMAPHORE -16
#define RENDER_ERROR_VULKAN_PIPELINE_LAYOUT -17
#define RENDER_ERROR_VULKAN_RENDER_PASS -18
#define RENDER_ERROR_VULKAN_SHADER_MODULE -19
#define RENDER_ERROR_VULKAN_GRAPHICS_PIPELINE -20
#define RENDER_ERROR_VULKAN_PIPELINE -21
#define RENDER_ERROR_VULKAN_IMAGE_VIEW -22
#define RENDER_ERROR_VULKAN_FRAMEBUFFER -23
#define RENDER_ERROR_VULKAN_COMMAND_POOL -24
#define RENDER_ERROR_VULKAN_COMMAND_BUFFER -25
#define RENDER_ERROR_VULKAN_BUFFER -26
#define RENDER_ERROR_VULKAN_VERTEX_DATA -27
#define RENDER_ERROR_VULKAN_MEMORY -28
#define RENDER_ERROR_VULKAN_MEMORY_MAP -29
#define RENDER_ERROR_VULKAN_COMMAND_BUFFER_BEGIN -30
#define RENDER_ERROR_VULKAN_COMMAND_BUFFER_END -31
#define RENDER_ERROR_VULKAN_SWAPCHAIN_RECREATE -32

int render_instance_init(struct render_instance *r, struct window *w);
void render_instance_deinit(struct render_instance *r);
int render_device_init(
  struct render_device *rd,
  struct render_instance *r,
  uint32_t device_id
);
void render_device_deinit(struct render_device *rd);
int render_pipeline_init(struct render_pipeline *rp, struct render_device *rd);
void render_pipeline_deinit(struct render_pipeline *rp);
void render_pipeline_update(struct render_pipeline *rp);

#endif
