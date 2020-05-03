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

#include "sized_types.h"
#include "window.h"

#include <vulkan/vulkan_core.h>

#define vkfunc(F) PFN_vk##F F
struct vk_functions {
  vkfunc(GetInstanceProcAddr);
  vkfunc(CreateInstance);
  vkfunc(DestroyInstance);
  vkfunc(EnumerateInstanceExtensionProperties);
  vkfunc(GetDeviceProcAddr);
  vkfunc(EnumeratePhysicalDevices);
};
#undef vkfunc

struct render {
  void *vklib;
  struct vk_functions vk;
  VkInstance instance;
  uint32_t n_devices;
  VkPhysicalDevice *phys_devices;
};

#define RENDER_ERROR_NONE 0
#define RENDER_ERROR_MEMORY -1
#define RENDER_ERROR_FILE -2
#define RENDER_ERROR_NULL -3
#define RENDER_ERROR_VULKAN_LOAD -4
#define RENDER_ERROR_VULKAN_INSTANCE -5
#define RENDER_ERROR_VULKAN_PREINST_LOAD -6
#define RENDER_ERROR_VULKAN_INST_LOAD -7
#define RENDER_ERROR_VULKAN_PHYSICAL_DEVICE -8
#define RENDER_ERROR_VULKAN_NO_DEVICES -9
#define RENDER_ERROR_VULKAN_INSTANCE_FUNC_LOAD -10
#define RENDER_ERROR_VULKAN_DEVICE_FUNC_LOAD -11
#define RENDER_ERROR_VULKAN_SURFACE -12
#define RENDER_ERROR_VULKAN_SURFACE_CAPS_IMAGE_USAGE -13
#define RENDER_ERROR_VULKAN_QUEUE_INDICES -14
#define RENDER_ERROR_VULKAN_QUEUE_INDEX_MISMATCH -15
#define RENDER_ERROR_VULKAN_CREATE_DEVICE -16
#define RENDER_ERROR_VULKAN_SURFACE_FORMAT -17
#define RENDER_ERROR_VULKAN_FORMAT_PROPERTIES_LINEAR -18
#define RENDER_ERROR_VULKAN_FORMAT_PROPERTIES_OPTIMAL -19
#define RENDER_ERROR_VULKAN_FORMAT_PROPERTIES_BUFFER -20
#define RENDER_ERROR_VULKAN_SURFACE_CAPABILITIES -21
#define RENDER_ERROR_VULKAN_SWAPCHAIN -22
#define RENDER_ERROR_VULKAN_SHADER_MODULE -23
#define RENDER_ERROR_VULKAN_SHADER_READ -24
#define RENDER_ERROR_VULKAN_DESCRIPTOR_SET_LAYOUT -25
#define RENDER_ERROR_VULKAN_PIPELINE_LAYOUT -26
#define RENDER_ERROR_VULKAN_CREATE_PIPELINE -27
#define RENDER_ERROR_VULKAN_RENDER_PASS -28
#define RENDER_ERROR_VULKAN_SWAPCHAIN_IMAGES -29
#define RENDER_ERROR_VULKAN_IMAGE_VIEW -30
#define RENDER_ERROR_VULKAN_FRAMEBUFFER -31
#define RENDER_ERROR_VULKAN_COMMAND_POOL -32
#define RENDER_ERROR_VULKAN_COMMAND_BUFFER -33
#define RENDER_ERROR_VULKAN_BUFFER -34
#define RENDER_ERROR_VULKAN_MEMORY -35
#define RENDER_ERROR_VULKAN_MEMORY_MAP -36
#define RENDER_ERROR_VULKAN_COMMAND_BUFFER_BEGIN -37
#define RENDER_ERROR_VULKAN_COMMAND_BUFFER_END -38
#define RENDER_ERROR_VULKAN_SEMAPHORE -39
#define RENDER_ERROR_VULKAN_ACQUIRE_IMAGE -40
#define RENDER_ERROR_VULKAN_QUEUE_SUBMIT -41
#define RENDER_ERROR_VULKAN_QUEUE_PRESENT -42
#define RENDER_ERROR_VULKAN_ENUMERATE_INSTANCE_EXTENSIONS -43
#define RENDER_ERROR_VULKAN_ENUMERATE_SUPPORTED_EXTENSIONS -44
#define RENDER_ERROR_VULKAN_UNSUPPORTED_EXTENSION -45

/* **************************************** */
/* render_<backend>.c */
int render_init(struct render *, struct window *);
void render_deinit(struct render *);
int render_configure(struct render *, uint16_t, uint16_t);
void render_update(struct render *);
/* **************************************** */

/* **************************************** */
/* render_<backend>_pipeline.c */
/* **************************************** */

#endif
