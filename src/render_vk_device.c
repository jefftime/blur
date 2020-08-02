/* Copyright 2020, Jeffery Stager
 *
 * This file is part of Tortuga
 *
 * Tortuga is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Tortuga is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Tortuga.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "render.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

static int get_queue_information(struct render_device *rd) {
  int graphics_set = 0, present_set = 0;
  uint32_t i, n_props;
  VkQueueFamilyProperties *props;
  VkResult result;

  vkGetPhysicalDeviceQueueFamilyProperties(
    rd->instance->pdevices[rd->device_id],
    &n_props,
    NULL
  );
  if (n_props == 0) goto err_n_props;
  props = malloc(sizeof(VkQueueFamilyProperties) * n_props);
  if (!props) goto err_props;
  vkGetPhysicalDeviceQueueFamilyProperties(
    rd->instance->pdevices[rd->device_id],
    &n_props,
    props
  );
  for (i = 0; i < n_props; ++i) {
    uint32_t present_support = 0;

    if (
      props[i].queueCount > 0
      && props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
    ) {
      graphics_set = 1;
      rd->graphics_index = i;
    }
    result = vkGetPhysicalDeviceSurfaceSupportKHR(
      rd->instance->pdevices[rd->device_id],
      i,
      rd->instance->surface,
      &present_support
    );
    if (result != VK_SUCCESS) goto err_surface_support;
    if (props[i].queueCount > 0 && present_support) {
      present_set = 1;
      rd->present_index = i;
    }
  }
  free(props);
  if (graphics_set && present_set) return RENDER_ERROR_NONE;
  return RENDER_ERROR_VULKAN_QUEUE_INDICES;

 err_surface_support:
  free(props);
 err_props:
 err_n_props:
  return RENDER_ERROR_VULKAN_QUEUE_INDICES;
}

static int create_device(struct render_device *rd) {
  char *extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  float priority = 1.0f;
  uint32_t n_queues = 0;
  VkDeviceQueueCreateInfo queue_infos[] = { { 0 }, { 0 } };
  VkDeviceCreateInfo create_info = { 0 };
  VkPhysicalDeviceFeatures features = { 0 };
  VkResult result;

  n_queues++;
  queue_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_infos[0].queueCount = 1;
  queue_infos[0].queueFamilyIndex = rd->graphics_index;
  queue_infos[0].pQueuePriorities = &priority;
  if (rd->present_index != rd->graphics_index) {
    n_queues++;
    queue_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_infos[1].queueCount = 1;
    queue_infos[1].queueFamilyIndex = rd->present_index;
    queue_infos[1].pQueuePriorities = &priority;
  }
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.pEnabledFeatures = &features;
  create_info.enabledExtensionCount = 1;
  create_info.ppEnabledExtensionNames = (const char *const *) extensions;
  create_info.queueCreateInfoCount = n_queues;
  create_info.pQueueCreateInfos = queue_infos;
  result = vkCreateDevice(
    rd->instance->pdevices[rd->device_id],
    &create_info,
    NULL,
    &rd->device
  );
  if (result != VK_SUCCESS) return RENDER_ERROR_VULKAN_DEVICE;
  return RENDER_ERROR_NONE;
}

static int load_device_functions(struct render_device *rd) {
#define vkfunc(F) \
  if (!(rd->F = (PFN_##F) vkGetDeviceProcAddr(rd->device, #F)))  \
    return RENDER_ERROR_VULKAN_DEVICE_FUNCTION

  /* Device functions */
  vkfunc(vkGetDeviceQueue);
  vkfunc(vkCreateSemaphore);
  vkfunc(vkDestroySemaphore);
  vkfunc(vkCreatePipelineLayout);
  vkfunc(vkDestroyPipelineLayout);
  vkfunc(vkCreateShaderModule);
  vkfunc(vkDestroyShaderModule);
  vkfunc(vkCreateRenderPass);
  vkfunc(vkDestroyRenderPass);
  vkfunc(vkCreateGraphicsPipelines);
  vkfunc(vkDestroyPipeline);
  vkfunc(vkCreateFramebuffer);
  vkfunc(vkDestroyFramebuffer);
  vkfunc(vkCreateImageView);
  vkfunc(vkDestroyImageView);
  vkfunc(vkCreateCommandPool);
  vkfunc(vkDestroyCommandPool);
  vkfunc(vkAllocateCommandBuffers);
  vkfunc(vkFreeCommandBuffers);
  vkfunc(vkBeginCommandBuffer);
  vkfunc(vkEndCommandBuffer);
  vkfunc(vkCmdBeginRenderPass);
  vkfunc(vkCmdEndRenderPass);
  vkfunc(vkCmdBindPipeline);
  vkfunc(vkCmdBindVertexBuffers);
  vkfunc(vkCmdBindIndexBuffer);
  vkfunc(vkCmdDrawIndexed);
  vkfunc(vkCreateDescriptorSetLayout);
  vkfunc(vkDestroyDescriptorSetLayout);
  /* Memory */
  vkfunc(vkCreateBuffer);
  vkfunc(vkDestroyBuffer);
  vkfunc(vkGetBufferMemoryRequirements);
  vkfunc(vkAllocateMemory);
  vkfunc(vkFreeMemory);
  vkfunc(vkBindBufferMemory);
  vkfunc(vkMapMemory);
  vkfunc(vkFlushMappedMemoryRanges);
  vkfunc(vkInvalidateMappedMemoryRanges);
  vkfunc(vkUnmapMemory);
  /* Present */
  vkfunc(vkAcquireNextImageKHR);
  vkfunc(vkQueueSubmit);
  vkfunc(vkQueuePresentKHR);
  vkfunc(vkQueueWaitIdle);
  return RENDER_ERROR_NONE;

#undef vkfunc
}

static VkSurfaceFormatKHR choose_format(
  uint32_t n_formats,
  VkSurfaceFormatKHR *formats
) {
  return formats[0];
}

static VkPresentModeKHR choose_present_mode(
  uint32_t n_present_modes,
  VkPresentModeKHR *present_modes
) {
  /* This is required to be supported, so we're just going to choose it
   * for now */
  return VK_PRESENT_MODE_FIFO_KHR;
}

static int create_swapchain(struct render_device *rd) {
  uint32_t n_formats, n_present_modes, n_images;
  VkSurfaceCapabilitiesKHR caps;
  VkSurfaceFormatKHR *formats;
  VkPresentModeKHR *present_modes;
  VkSwapchainCreateInfoKHR create_info = { 0 };
  VkResult result;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    rd->instance->pdevices[rd->device_id],
    rd->instance->surface,
    &caps
  );
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    rd->instance->pdevices[rd->device_id],
    rd->instance->surface,
    &n_formats,
    NULL
  );
  if (n_formats == 0) goto err_n_formats;
  formats = malloc(sizeof(VkSurfaceFormatKHR) * n_formats);
  if (!formats) goto err_formats;
  vkGetPhysicalDeviceSurfaceFormatsKHR(
    rd->instance->pdevices[rd->device_id],
    rd->instance->surface,
    &n_formats,
    formats
  );
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    rd->instance->pdevices[rd->device_id],
    rd->instance->surface,
    &n_present_modes,
    NULL
  );
  if (n_present_modes == 0) goto err_n_present_modes;
  present_modes = malloc(sizeof(VkPresentModeKHR) * n_present_modes);
  if (!present_modes) goto err_present_modes;
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    rd->instance->pdevices[rd->device_id],
    rd->instance->surface,
    &n_present_modes,
    present_modes
  );
  rd->surface_format = choose_format(n_formats, formats);
  create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  create_info.surface = rd->instance->surface;
  create_info.minImageCount = caps.minImageCount;
  create_info.imageFormat = rd->surface_format.format;
  create_info.imageColorSpace = rd->surface_format.colorSpace;
  create_info.imageExtent = caps.currentExtent;
  create_info.imageArrayLayers = 1;
  create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  if (rd->graphics_index == rd->present_index) {
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.queueFamilyIndexCount = 0;
    create_info.pQueueFamilyIndices = NULL;
  } else {
    uint32_t indices[2];

    indices[0] = rd->graphics_index;
    indices[1] = rd->present_index;
    create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    create_info.queueFamilyIndexCount = 2;
    create_info.pQueueFamilyIndices = indices;
  }
  create_info.preTransform = caps.currentTransform;
  create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  create_info.presentMode = choose_present_mode(
    n_present_modes,
    present_modes
  );
  create_info.clipped = VK_TRUE;
  create_info.oldSwapchain = VK_NULL_HANDLE;
  result = vkCreateSwapchainKHR(
    rd->device,
    &create_info,
    NULL,
    &rd->swapchain
  );
  if (result != VK_SUCCESS) goto err_swapchain;
  rd->swap_extent = caps.currentExtent;
  vkGetSwapchainImagesKHR(
    rd->device,
    rd->swapchain,
    &n_images,
    NULL
  );
  if (n_images == 0) goto err_swapchain_images;
  rd->swapchain_images = malloc(sizeof(VkImage) * n_images);
  if (!rd->swapchain_images) goto err_swapchain_images;
  vkGetSwapchainImagesKHR(
    rd->device,
    rd->swapchain,
    &n_images,
    rd->swapchain_images
  );
  rd->n_swapchain_images = n_images;
  free(present_modes);
  free(formats);
  return RENDER_ERROR_NONE;

 err_swapchain_images:
 err_swapchain:
  free(present_modes);
 err_present_modes:
 err_n_present_modes:
  free(formats);
 err_formats:
 err_n_formats:
  return RENDER_ERROR_VULKAN_SWAPCHAIN;
}

static int create_semaphores(struct render_device *rd) {
  VkSemaphoreCreateInfo create_info = { 0 };
  VkResult result;

  create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  result = rd->vkCreateSemaphore(
    rd->device,
    &create_info,
    NULL,
    &rd->image_semaphore
  );
  if (result != VK_SUCCESS) goto err_image_semaphore;
  result = rd->vkCreateSemaphore(
    rd->device,
    &create_info,
    NULL,
    &rd->render_semaphore
  );
  if (result != VK_SUCCESS) goto err_render_semaphore;
  return RENDER_ERROR_NONE;

 err_render_semaphore:
  rd->vkDestroySemaphore(rd->device, rd->image_semaphore, NULL);
 err_image_semaphore:
  return RENDER_ERROR_VULKAN_SEMAPHORE;
}

/* **************************************** */
/* Public */
/* **************************************** */

int render_device_init(
  struct render_device *rd,
  struct render_instance *ri,
  uint32_t device_id
) {
  int err;
  VkBufferUsageFlags usage_flags;

  if (!rd) return RENDER_ERROR_NULL;
  if (!ri) return RENDER_ERROR_NULL;
  if (device_id > ri->n_pdevices) return RENDER_ERROR_VULKAN_INVALID_DEVICE;
  memset(rd, 0, sizeof(struct render_device));
  rd->instance = ri;
  rd->device_id = device_id;
  vkGetPhysicalDeviceProperties(
    rd->instance->pdevices[rd->device_id],
    &rd->properties
  );
  vkGetPhysicalDeviceFeatures(
    rd->instance->pdevices[rd->device_id],
    &rd->features
  );
  vkGetPhysicalDeviceMemoryProperties(
    rd->instance->pdevices[rd->device_id],
    &rd->memory_properties
  );
  chkerrg(err = get_queue_information(rd), err_queue);
  chkerrg(err = create_device(rd), err_device);
  chkerrg(err = load_device_functions(rd), err_load_functions);
  usage_flags =
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  chkerrg(
    err = render_memory_init(
      &rd->memory,
      rd,
      usage_flags,
      MB_TO_BYTES(2)
    ),
    err_memory
  );
  chkerrg(err = create_swapchain(rd), err_swapchain);
  chkerrg(err = create_semaphores(rd), err_semaphore);
  rd->vkGetDeviceQueue(
    rd->device,
    (uint32_t) rd->graphics_index,
    0,
    &rd->graphics_queue
  );
  rd->vkGetDeviceQueue(
    rd->device,
    (uint32_t) rd->present_index,
    0,
    &rd->present_queue
  );
  return RENDER_ERROR_NONE;

 err_semaphore:
  free(rd->swapchain_images);
  vkDestroySwapchainKHR(rd->device, rd->swapchain, NULL);
 err_swapchain:
  render_memory_deinit(&rd->memory);
 err_memory:
 err_load_functions:
  vkDestroyDevice(rd->device, NULL);
 err_device:
 err_queue:
  return err;
}

void render_device_deinit(struct render_device *rd) {
  rd->vkDestroyBuffer(rd->device, rd->memory.buffer, NULL);
  rd->vkFreeMemory(rd->device, rd->memory.memory, NULL);
  rd->vkDestroySemaphore(rd->device, rd->image_semaphore, NULL);
  rd->vkDestroySemaphore(rd->device, rd->render_semaphore, NULL);
  free(rd->swapchain_images);
  vkDestroySwapchainKHR(rd->device, rd->swapchain, NULL);
  vkDestroyDevice(rd->device, NULL);
}

int render_device_recreate_swapchain(struct render_device *rd) {
  if (!rd) return RENDER_ERROR_NULL;
  free(rd->swapchain_images);
  vkDestroySwapchainKHR(rd->device, rd->swapchain, NULL);
  chkerrg(create_swapchain(rd), err_swapchain);
  return RENDER_ERROR_NONE;

 err_swapchain:
  return RENDER_ERROR_VULKAN_SWAPCHAIN_RECREATE;
}
