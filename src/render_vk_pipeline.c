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

/* static int get_queue_props(struct render_pipeline *rp) { */
/*   uint32_t n_props; */

/*   vkGetPhysicalDeviceQueueFamilyProperties( */
/*     rp->ctx->phys_devices[rp->phys_device], */
/*     &n_props, */
/*     NULL */
/*   ); */
/*   if (n_props == 0) return RENDER_ERROR_VULKAN_PHYSICAL_DEVICE; */
/*   rp->n_queue_props = n_props; */
/*   rp->queue_props = malloc(sizeof(VkQueueFamilyProperties) * n_props); */
/*   if (!rp->queue_props) return RENDER_ERROR_MEMORY; */
/*   vkGetPhysicalDeviceQueueFamilyProperties( */
/*     rp->ctx->phys_devices[rp->phys_device], */
/*     &n_props, */
/*     rp->queue_props */
/*   ); */
/*   return RENDER_ERROR_NONE; */
/* } */

/* static int get_queue_indices(struct render_pipeline *rp) { */
/*   int graphics_isset = 0; */
/*   int present_isset = 0; */
/*   uint32_t i; */
/*   VkResult result; */

/*   for (i = 0; i < rp->n_queue_props; ++i) { */
/*     uint32_t present_support = 0; */

/*     /\* Check graphics support *\/ */
/*     if ( */
/*       (rp->queue_props[i].queueCount > 0) */
/*       && (rp->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) */
/*     ) { */
/*       graphics_isset = 1; */
/*       rp->queue_index_graphics = i; */
/*     } */
/*     /\* Check present support *\/ */
/*     result = vkGetPhysicalDeviceSurfaceSupportKHR( */
/*       rp->ctx->phys_devices[rp->phys_device], */
/*       (uint32_t) i, */
/*       rp->ctx->surface, */
/*       &present_support */
/*     ); */
/*     if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_QUEUE_INDICES; */
/*     if ((rp->queue_props[i].queueCount > 0) && present_support) { */
/*       present_isset = 1; */
/*       rp->queue_index_present = i; */
/*     } */
/*   } */
/*   if (graphics_isset && present_isset) return RENDER_ERROR_NONE; */
/*   return RENDER_ERROR_VULKAN_QUEUE_INDICES; */
/* } */

/* static int create_device(struct render_pipeline *rp) { */
/*   char *extensions[] = { "VK_KHR_swapchain" }; */
/*   float queue_priority = 1.0f; */
/*   VkDeviceQueueCreateInfo queue_create_infos[] = { { 0 }, { 0 } }; */
/*   VkDeviceCreateInfo create_info = { 0 }; */
/*   VkResult result; */

/*   /\* TODO: support separate graphics and present queues since */
/*    * curretly this is out of spec *\/ */
/*   if (rp->queue_index_present != rp->queue_index_graphics) { */
/*     return RENDER_ERROR_VULKAN_QUEUE_INDEX_MISMATCH; */
/*   } */
/*   /\* Graphics queue *\/ */
/*   queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; */
/*   queue_create_infos[0].queueFamilyIndex = rp->queue_index_graphics; */
/*   queue_create_infos[0].queueCount = 1; */
/*   queue_create_infos[0].pQueuePriorities = &queue_priority; */
/*   /\* Present queue *\/ */
/*   queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; */
/*   queue_create_infos[1].queueFamilyIndex = rp->queue_index_present; */
/*   queue_create_infos[1].queueCount = 1; */
/*   queue_create_infos[1].pQueuePriorities = &queue_priority; */
/*   create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; */
/*   create_info.queueCreateInfoCount = 1; */
/*   create_info.pQueueCreateInfos = queue_create_infos; */
/*   create_info.enabledExtensionCount = 1; */
/*   create_info.ppEnabledExtensionNames = (const char *const *) extensions; */
/*   /\* result = vkCreateDevice( *\/ */
/*   /\*   rp->ctx->phys_devices[rp->phys_device], *\/ */
/*   /\*   &create_info, *\/ */
/*   /\*   NULL, *\/ */
/*   /\*   &rp->device *\/ */
/*   /\* ); *\/ */
/*   /\* if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_CREATE_DEVICE; *\/ */
/*   return RENDER_ERROR_NONE; */
/* } */

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
  rp->phys_device = device;
  /* chkerrf(get_queue_props(rp), render_deinit_pipeline(rp)); */
  /* chkerrf(get_queue_indices(rp), render_deinit_pipeline(rp)); */
  /* chkerrf(create_device(rp), render_deinit_pipeline(rp)); */
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
  free(rp->queue_props);
}
