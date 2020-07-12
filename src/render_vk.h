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

#ifndef RENDER_VK_H
#define RENDER_VK_H

#define VK_NO_PROTOTYPES
#include <vulkan/vulkan_core.h>
#include <vulkan/vk_platform.h>

#ifdef PLATFORM_LINUX
#include <xcb/xcb.h>
#include <vulkan/vulkan_xcb.h>
#endif

#define vkfunc(F) PFN_##F F

/* Instance */
vkfunc(vkGetInstanceProcAddr);
vkfunc(vkCreateInstance);
vkfunc(vkDestroyInstance);
vkfunc(vkEnumerateInstanceExtensionProperties);
vkfunc(vkCreateXcbSurfaceKHR);
vkfunc(vkDestroySurfaceKHR);
vkfunc(vkEnumeratePhysicalDevices);
vkfunc(vkGetDeviceProcAddr);
vkfunc(vkCreateDevice);
vkfunc(vkDestroyDevice);
vkfunc(vkGetPhysicalDeviceQueueFamilyProperties);
vkfunc(vkGetPhysicalDeviceSurfaceSupportKHR);
vkfunc(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
vkfunc(vkGetPhysicalDeviceSurfaceFormatsKHR);
vkfunc(vkGetPhysicalDeviceSurfacePresentModesKHR);
vkfunc(vkCreateSwapchainKHR);
vkfunc(vkDestroySwapchainKHR);
vkfunc(vkGetSwapchainImagesKHR);
vkfunc(vkGetPhysicalDeviceMemoryProperties);
vkfunc(vkGetPhysicalDeviceProperties);

struct render_instance {
  struct window *window;
  void *vk_handle;
  VkInstance instance;
  VkSurfaceKHR surface;
  size_t n_pdevices;
  VkPhysicalDevice *pdevices;
};

struct render_device {
  struct render_instance *instance;
  uint32_t device_id;
  uint32_t graphics_index;
  uint32_t present_index;
  VkDevice device;
  VkSurfaceFormatKHR surface_format;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkExtent2D swap_extent;
  VkSwapchainKHR swapchain;
  uint32_t n_swapchain_images;
  VkImage *swapchain_images;
  VkSemaphore image_semaphore;
  VkSemaphore render_semaphore;
  VkPhysicalDeviceProperties properties;

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
};

struct render_pass {
  struct render_device *device;
  VkRenderPass render_pass;
  VkPipeline pipeline;
  VkImageView *image_views;
  VkFramebuffer *framebuffers;
  VkCommandPool command_pool;
  VkCommandBuffer *command_buffers;
  VkBuffer vertex_buffer;
  VkBuffer index_buffer;
  VkDeviceMemory vertex_memory;
  VkDeviceMemory index_memory;
};

struct render_data {
  struct render_pass *pipeline;
  VkBuffer vertex_buffer;
  VkBuffer index_buffer;
};

struct render_shader {
  VkShaderModule vert_module;
  VkShaderModule frag_module;
};

#undef vkfunc

/* **************************************** */
/* render_vk_device.c */
int render_device_recreate_swapchain(struct render_device *rd);
/* **************************************** */

/* **************************************** */
/* render_vk_memory.c */
int create_buffer(
  struct render_pass *rp,
  VkBuffer *out_buf,
  size_t size,
  VkBufferUsageFlags flags
);
int alloc_buffer(
  struct render_pass *rp,
  VkBuffer buf,
  VkDeviceMemory *out_mem
);
int write_data(
  struct render_pass *rp,
  VkDeviceMemory mem,
  size_t size,
  void *data
);
/* **************************************** */

#endif
